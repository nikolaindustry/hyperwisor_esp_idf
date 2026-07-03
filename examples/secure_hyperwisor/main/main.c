/*
 * secure_hyperwisor — HSC v1 (Hyperwisor Secure Channel) example.
 *
 * The device generates a P-256 keypair ON-CHIP at first boot (the private key
 * never leaves the device) and proves its identity to the secured relay with a
 * signed challenge on every connect. No secret is ever transmitted.
 *
 * Setup:
 *   1. Point the library at your secured relay:
 *        idf.py menuconfig  ->  Hyperwisor  ->  WebSocket server hostname
 *      (CONFIG_HYPERWISOR_WS_HOST), or set it in sdkconfig.defaults.
 *   2. Call hyperwisor_enable_security() AFTER init() and BEFORE start().
 *   3. Register this device's PUBLIC key with the platform once during
 *      onboarding (print it, or call hyperwisor_hsc_register()).
 */

#include "hyperwisor.h"
#include "esp_log.h"

static const char *TAG = "SECURE_HYPER";

void app_main(void)
{
    /* 1. init the library (loads NVS, prepares state) */
    hyperwisor_init();

    /* 2. enable HSC — generates/loads the on-chip key and turns on the
     *    handshake. Must be called before start(). */
    if (hyperwisor_enable_security() == ESP_OK) {
        char pub[128];
        if (hyperwisor_get_public_key_b64(pub, sizeof(pub)) == ESP_OK) {
            ESP_LOGW(TAG, "Register this device public key with the platform:");
            ESP_LOGW(TAG, "%s", pub);
        }
    }

    /* 3. provision once at first boot (or use SoftAP setup) */
    if (!hyperwisor_has_credentials()) {
        ESP_LOGW(TAG, "No credentials — use SoftAP setup or set them in code.");
        /* hyperwisor_set_credentials("MySSID", "MyPass",
         *                            "device-uuid", "user-uuid"); */
    }

    /* Optional: register the public key now if you have the user's auth token.
     * hyperwisor_hsc_register("https://<project>.supabase.co/functions/v1",
     *                         user_auth_token); */

    /* 4. spawn the core task — it connects and does the HSC handshake
     *    automatically. Watch for "HSC: authenticated" in the log. */
    hyperwisor_start();
}
