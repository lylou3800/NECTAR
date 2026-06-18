---
description: Compile et flashe le firmware NECTAR sur la carte ESP32-S3 (ESP-IDF v6.0.1)
---

Compile (si nécessaire) et flashe le firmware NECTAR sur la carte ESP32-S3 connectée.

Exécute la commande suivante **en un seul appel PowerShell** (l'environnement ESP-IDF doit être activé dans le même appel), depuis la racine du projet `firmware/` :

```
& 'C:\Espressif\tools\Microsoft.v6.0.1.PowerShell_profile.ps1'; idf.py flash $ARGUMENTS
```

Règles :
- `$ARGUMENTS` permet de préciser un port, par ex. `/flash -p COM5`. Sans argument, le port est auto-détecté.
- N'ajoute **pas** `monitor` (il est interactif et bloque le terminal). Pour la console série, l'utilisateur lance `idf.py monitor` lui-même (ou `/flash` puis `idf.py monitor`).
- Donne un timeout généreux à l'appel (le flash peut prendre 1 à 3 minutes).
- Si le flash échoue avec une erreur de port (« could not open port », « No serial data received », port introuvable) : liste les ports série disponibles avec `[System.IO.Ports.SerialPort]::GetPortNames()` en PowerShell, puis demande à l'utilisateur quel port utiliser et relance avec `-p <COMx>`.
- Rapporte le résultat : succès (« Hash of data verified », « Hard resetting ») ou l'erreur clé.
