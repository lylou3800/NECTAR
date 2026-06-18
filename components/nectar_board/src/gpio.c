#include "gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TCA9548A_ADDR 0x70  // Adresse du multiplexeur I2C TCA9548A
#define MCP23017_ADDR 0x20  // Adresse du MCP23017 (par défaut)
#define MCP23017_IODIRA 0x00  // Registre configuration port A
#define MCP23017_GPIOA 0x12   // Registre lecture port A
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_FREQ_HZ 100000
#define VERRE_GPIO_PIN 0      // GPA0 pour le capteur de verre

static const char *TAG = "GPIO";
static bool i2c_initialized = false;
static bool verre = false;
static esp_timer_handle_t pompe_timer = NULL;
static uint32_t pompe_duration_ms = 0;  // Durée en millisecondes

/**
 * @brief Initialise le bus I2C pour communiquer avec le TCA9548A
 */
static esp_err_t init_i2c(void) {
    if (i2c_initialized) {
        return ESP_OK;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_8,      // SDA sur GPIO 21
        .scl_io_num = GPIO_NUM_9,      // SCL sur GPIO 22
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur configuration I2C: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur installation driver I2C: %s", esp_err_to_name(err));
        return err;
    }

    i2c_initialized = true;
    ESP_LOGI(TAG, "Bus I2C initialisé avec succès");
    return ESP_OK;
}

/**
 * @brief Callback du timer pour éteindre la pompe automatiquement
 */
static void pompe_timer_callback(void *arg) {
    ESP_LOGI(TAG, "Timer expiration - Arrêt de la pompe");
    // Envoie la commande I2C pour éteindre
    uint8_t data = 0x00;  // Désactiver
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (TCA9548A_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Pompe éteinte automatiquement");
    } else {
        ESP_LOGE(TAG, "Erreur lors de l'extinction de la pompe: %s", esp_err_to_name(err));
    }
}

/**
 * @brief Contrôle la pompe via le TCA9548A en I2C
 * @param enable true pour activer, false pour désactiver
 * @param duration_seconds Durée en secondes avant d'éteindre automatiquement (0 = pas de timer)
 */
void controlPompe(bool enable) {
    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Impossible d'initialiser l'I2C");
        return;
    }

    // Valeur à envoyer: 0x01 pour activer, 0x00 pour désactiver
    uint8_t data = enable ? 0x01 : 0x00;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (TCA9548A_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);

    err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Pompe %s avec succès", enable ? "activée" : "désactivée");
    } else {
        ESP_LOGE(TAG, "Erreur lors du contrôle de la pompe: %s", esp_err_to_name(err));
        return;
    }

    // Gestion du timer pour éteindre automatiquement
    if (enable && pompe_duration_ms > 0) {
        // Si un timer existe déjà, l'arrêter
        if (pompe_timer != NULL) {
            esp_timer_stop(pompe_timer);
            esp_timer_delete(pompe_timer);
        }

        // Créer un nouveau timer
        esp_timer_create_args_t timer_args = {
            .callback = pompe_timer_callback,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "pompe_timer"
        };

        err = esp_timer_create(&timer_args, &pompe_timer);
        if (err == ESP_OK) {
            // Démarrer le timer (durée en microsecondes)
            esp_timer_start_once(pompe_timer, pompe_duration_ms * 1000);
            ESP_LOGI(TAG, "Timer démarré: pompe s'éteindra dans %d ms", pompe_duration_ms);
        } else {
            ESP_LOGE(TAG, "Erreur création timer: %s", esp_err_to_name(err));
        }
    } else if (!enable) {
        // Arrêter le timer si on éteint la pompe manuellement
        if (pompe_timer != NULL) {
            esp_timer_stop(pompe_timer);
            esp_timer_delete(pompe_timer);
            pompe_timer = NULL;
            ESP_LOGI(TAG, "Timer arrêté");
        }
    }
}

/**
 * @brief Lance un cycle de service pour servir une boisson
 * @param secondes Durée d'activation de la pompe en secondes
 */
void servirBoisson(uint32_t secondes) {
    ESP_LOGI(TAG, "=== Démarrage du cycle de service ===");
    ESP_LOGI(TAG, "Durée de service: %d secondes", secondes);

    verre=true;

    // Configure la durée du timer (conversion secondes -> millisecondes)
    if (verre){
    setPompeDuration(secondes * 1000);
    }else{
    setPompeDuration(0);
    }

    // Active la pompe (le timer s'arrêtera automatiquement après la durée)
    controlPompe(true);
    
    // Les autres vérifications de paramètres se feront ici plus tard
    // TODO: ajouter les vérifications de capteurs, poids, etc.
}

/**
 * @brief Écrit dans un registre du MCP23017
 * @param reg Adresse du registre
 * @param value Valeur à écrire
 * @return esp_err_t
 */
static esp_err_t mcp23017_write_register(uint8_t reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return err;
}

/**
 * @brief Lit un registre du MCP23017
 * @param reg Adresse du registre
 * @param value Pointeur pour stocker la valeur lue
 * @return esp_err_t
 */
static esp_err_t mcp23017_read_register(uint8_t reg, uint8_t *value) {
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Écrire l'adresse du registre à lire
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (err != ESP_OK) {
        return err;
    }

    // Lire la valeur du registre
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MCP23017_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, value, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    return err;
}

/**
 * @brief Initialise le capteur de verre (MCP23017)
 */
static esp_err_t init_mcp23017(void) {
    esp_err_t err = init_i2c();
    if (err != ESP_OK) {
        return err;
    }

    // Configurer GPA0 en entrée (0xFF = tous les ports A en entrée)
    err = mcp23017_write_register(MCP23017_IODIRA, 0xFF);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "MCP23017 initialisé (capteur de verre)");
    } else {
        ESP_LOGE(TAG, "Erreur initialisation MCP23017: %s", esp_err_to_name(err));
    }

    return err;
}

/**
 * @brief Vérifie si le verre est présent
 * @return true si verre détecté, false sinon
 */
bool checkVerre(void) {
    esp_err_t err = init_mcp23017();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture capteur verre");
        return false;
    }

    uint8_t gpio_value = 0;
    err = mcp23017_read_register(MCP23017_GPIOA, &gpio_value);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur lecture GPIO verre: %s", esp_err_to_name(err));
        return false;
    }

    // Extraire le bit du capteur de verre (GPA0)
    bool verre_present = (gpio_value & (1 << VERRE_GPIO_PIN)) != 0;
    
    ESP_LOGI(TAG, "État verre: %s (GPIO value: 0x%02X)", 
             verre_present ? "PRÉSENT" : "ABSENT", gpio_value);
    
    return verre_present;
}

/**
 * @brief Définis la durée du timer avant extinction de la pompe
 * @param duration_ms Durée en millisecondes (0 = désactiver le timer)
 */
void setPompeDuration(uint32_t duration_ms) {
    pompe_duration_ms = duration_ms;
    ESP_LOGI(TAG, "Durée pompe configurée: %d ms", duration_ms);
}

/**
 * @brief Lit le poids (fonction stub)
 */
float lirePoid(void) {
    return 0.0f;
}

/**
 * @brief Lit la tension du poids en I2C (fonction stub)
 */
float lireTensionPoidI2C(void) {
    return 0.0f;
}
