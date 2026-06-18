#include "gpio.h"

#include <inttypes.h>

#include "board_display.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TCA9548A_ADDR 0x70   // Multiplexeur I2C TCA9548A (commande pompe)
#define MCP23017_ADDR 0x20   // MCP23017 (capteur de verre)
#define MCP23017_IODIRA 0x00 // Registre configuration port A
#define MCP23017_GPIOA 0x12  // Registre lecture port A
#define VERRE_GPIO_PIN 0     // GPA0 pour le capteur de verre

#define I2C_DEV_FREQ_HZ 100000
#define I2C_TIMEOUT_MS 1000

static const char *TAG = "GPIO";

/* Le bus I2C est créé et possédé par board_display (nouveau driver i2c_master,
 * GPIO 8/9, I2C_NUM_0). On y ajoute simplement nos deux périphériques : on ne
 * réinstalle PAS de driver (cela entrerait en conflit avec l'écran/tactile). */
static i2c_master_dev_handle_t s_tca_dev = NULL;  // pompe (TCA9548A)
static i2c_master_dev_handle_t s_mcp_dev = NULL;  // capteur verre (MCP23017)
static bool s_mcp_configured = false;

static esp_timer_handle_t pompe_timer = NULL;
static uint32_t pompe_duration_ms = 0;  // Durée en millisecondes

/**
 * @brief Attache nos deux périphériques au bus I2C partagé (une seule fois).
 */
static esp_err_t ensure_devices(void)
{
    i2c_master_bus_handle_t bus = board_i2c_bus();

    if (bus == NULL) {
        ESP_LOGE(TAG, "Bus I2C non pret (board_display_init non execute ?)");
        return ESP_ERR_INVALID_STATE;
    }

    if (s_tca_dev == NULL) {
        const i2c_device_config_t tca_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = TCA9548A_ADDR,
            .scl_speed_hz = I2C_DEV_FREQ_HZ,
        };
        esp_err_t err = i2c_master_bus_add_device(bus, &tca_cfg, &s_tca_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Ajout TCA9548A (0x%02X) echoue: %s", TCA9548A_ADDR, esp_err_to_name(err));
            s_tca_dev = NULL;
            return err;
        }
    }

    if (s_mcp_dev == NULL) {
        const i2c_device_config_t mcp_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = MCP23017_ADDR,
            .scl_speed_hz = I2C_DEV_FREQ_HZ,
        };
        esp_err_t err = i2c_master_bus_add_device(bus, &mcp_cfg, &s_mcp_dev);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Ajout MCP23017 (0x%02X) echoue: %s", MCP23017_ADDR, esp_err_to_name(err));
            s_mcp_dev = NULL;
            return err;
        }
    }

    return ESP_OK;
}

/**
 * @brief Callback du timer pour éteindre la pompe automatiquement.
 */
static void pompe_timer_callback(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "Timer expiration - Arret de la pompe");

    if (s_tca_dev == NULL) {
        return;
    }

    const uint8_t data = 0x00;  // Désactiver
    esp_err_t err = i2c_master_transmit(s_tca_dev, &data, 1, I2C_TIMEOUT_MS);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Pompe eteinte automatiquement");
    } else {
        ESP_LOGE(TAG, "Erreur extinction pompe: %s", esp_err_to_name(err));
    }
}

/**
 * @brief Contrôle la pompe via le TCA9548A en I2C.
 * @param enable true pour activer, false pour désactiver
 */
void controlPompe(bool enable)
{
    if (ensure_devices() != ESP_OK) {
        ESP_LOGE(TAG, "Impossible d'initialiser l'I2C pompe");
        return;
    }

    // Valeur à envoyer: 0x01 pour activer, 0x00 pour désactiver
    const uint8_t data = enable ? 0x01 : 0x00;
    esp_err_t err = i2c_master_transmit(s_tca_dev, &data, 1, I2C_TIMEOUT_MS);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "POMPE %s -> I2C addr 0x%02X = 0x%02X",
                 enable ? "ACTIVEE" : "DESACTIVEE", TCA9548A_ADDR, data);
    } else {
        ESP_LOGE(TAG, "Erreur controle pompe: %s", esp_err_to_name(err));
        return;
    }

    // Gestion du timer d'extinction automatique
    if (enable && pompe_duration_ms > 0) {
        if (pompe_timer != NULL) {
            esp_timer_stop(pompe_timer);
            esp_timer_delete(pompe_timer);
            pompe_timer = NULL;
        }

        const esp_timer_create_args_t timer_args = {
            .callback = pompe_timer_callback,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "pompe_timer",
        };

        err = esp_timer_create(&timer_args, &pompe_timer);
        if (err == ESP_OK) {
            esp_timer_start_once(pompe_timer, (uint64_t)pompe_duration_ms * 1000);
            ESP_LOGI(TAG, "Timer demarre: extinction dans %" PRIu32 " ms", pompe_duration_ms);
        } else {
            ESP_LOGE(TAG, "Erreur creation timer: %s", esp_err_to_name(err));
        }
    } else if (!enable) {
        if (pompe_timer != NULL) {
            esp_timer_stop(pompe_timer);
            esp_timer_delete(pompe_timer);
            pompe_timer = NULL;
            ESP_LOGI(TAG, "Timer arrete");
        }
    }
}

/**
 * @brief Lance un cycle de service pour servir une boisson.
 * @param secondes Durée d'activation de la pompe en secondes
 */
void servirBoisson(uint32_t secondes)
{
    ESP_LOGI(TAG, "=== Demarrage du cycle de service ===");
    ESP_LOGI(TAG, "Duree de service: %" PRIu32 " secondes", secondes);

    if (!checkVerre()) {
        ESP_LOGW(TAG, "Aucun verre detecte -> service annule");
        return;
    }

    // Configure la durée du timer (conversion secondes -> millisecondes)
    setPompeDuration(secondes * 1000);

    // Active la pompe (le timer l'arrêtera automatiquement après la durée)
    controlPompe(true);
}

/**
 * @brief Écrit dans un registre du MCP23017.
 */
static esp_err_t mcp23017_write_register(uint8_t reg, uint8_t value)
{
    const uint8_t buf[2] = {reg, value};
    return i2c_master_transmit(s_mcp_dev, buf, sizeof(buf), I2C_TIMEOUT_MS);
}

/**
 * @brief Lit un registre du MCP23017.
 */
static esp_err_t mcp23017_read_register(uint8_t reg, uint8_t *value)
{
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_transmit_receive(s_mcp_dev, &reg, 1, value, 1, I2C_TIMEOUT_MS);
}

/**
 * @brief Initialise le capteur de verre (MCP23017) : port A en entrée.
 */
static esp_err_t init_mcp23017(void)
{
    esp_err_t err = ensure_devices();
    if (err != ESP_OK) {
        return err;
    }

    if (s_mcp_configured) {
        return ESP_OK;
    }

    // Configurer le port A en entrée (0xFF = tous les bits en entrée)
    err = mcp23017_write_register(MCP23017_IODIRA, 0xFF);
    if (err == ESP_OK) {
        s_mcp_configured = true;
        ESP_LOGI(TAG, "MCP23017 initialise (capteur de verre)");
    } else {
        ESP_LOGE(TAG, "Erreur init MCP23017: %s", esp_err_to_name(err));
    }

    return err;
}

/**
 * @brief Vérifie si le verre est présent.
 * @return true si verre détecté, false sinon
 */
bool checkVerre(void)
{
    if (init_mcp23017() != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture capteur verre");
        return false;
    }

    uint8_t gpio_value = 0;
    esp_err_t err = mcp23017_read_register(MCP23017_GPIOA, &gpio_value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture GPIO verre: %s", esp_err_to_name(err));
        return false;
    }

    const bool verre_present = (gpio_value & (1u << VERRE_GPIO_PIN)) != 0;
    ESP_LOGI(TAG, "Etat verre: %s (GPIOA: 0x%02X)",
             verre_present ? "PRESENT" : "ABSENT", gpio_value);

    return verre_present;
}

/**
 * @brief Définit la durée du timer avant extinction de la pompe.
 * @param duration_ms Durée en millisecondes (0 = pas de timer auto)
 */
void setPompeDuration(uint32_t duration_ms)
{
    pompe_duration_ms = duration_ms;
    ESP_LOGI(TAG, "Duree pompe configuree: %" PRIu32 " ms", duration_ms);
}

/**
 * @brief Lit le poids (stub).
 */
float lirePoid(void)
{
    return 0.0f;
}

/**
 * @brief Lit la tension du poids en I2C (stub).
 */
float lireTensionPoidI2C(void)
{
    return 0.0f;
}
