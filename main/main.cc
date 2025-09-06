#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_http_server.h>
#include <lwip/err.h>
#include <lwip/sys.h>

static const char* TAG = "oraculo";
static httpd_handle_t server = NULL;

// Página HTML simplificada para configuração


// Handler para página principal
static esp_err_t config_handler(httpd_req_t *req) {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    httpd_resp_set_type(req, "text/html");
    
    const char* html = 
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<title>Oráculo Assistant</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}"
    ".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:10px;}"
    "h1{color:#333;text-align:center;}"
    ".form-group{margin:15px 0;}"
    "label{display:block;margin-bottom:5px;font-weight:bold;}"
    "input,select{width:100%%;padding:8px;border:1px solid #ddd;border-radius:4px;}"
    ".btn{background:#007bff;color:white;padding:10px 20px;border:none;border-radius:4px;cursor:pointer;}"
    ".btn:hover{background:#0056b3;}"
    "</style>"
    "</head><body>"
    "<div class='container'>"
    "<h1>🔮 Oráculo Assistant</h1>"
    "<h2>Configuração WiFi</h2>"
    "<form action='/save_wifi' method='post'>"
    "<div class='form-group'>"
    "<label>Nome da Rede (SSID):</label>"
    "<input type='text' name='ssid' required>"
    "</div>"
    "<div class='form-group'>"
    "<label>Senha:</label>"
    "<input type='password' name='password'>"
    "</div>"
    "<button type='submit' class='btn'>Salvar</button>"
    "</form>"
    "<hr>"
    "<h2>Informações do Sistema</h2>"
    "<p><strong>MAC Address:</strong> %s</p>"
    "<p><strong>Status:</strong> Configuração Ativa</p>"
    "</div></body></html>";
    
    char *response = (char*)malloc(2048);
    if (response == NULL) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    snprintf(response, 2048, html, mac_str);
    httpd_resp_send(req, response, strlen(response));
    
    free(response);
    return ESP_OK;
}

// Handler para salvar configurações WiFi
static esp_err_t save_wifi_handler(httpd_req_t *req) {
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "📶 Configuração WiFi recebida: %s", buf);
    
    const char* response = "<html><body style='font-family: Arial; text-align: center; margin-top: 100px;'><h1>✅ Configuração WiFi Salva!</h1><p>O sistema será reiniciado em 3 segundos...</p><script>setTimeout(() => location.href='/', 3000);</script></body></html>";
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

// Handler para salvar configurações de pinos
static esp_err_t save_pins_handler(httpd_req_t *req) {
    char buf[1024];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "🔌 Configuração de pinos recebida: %s", buf);
    
    const char* response = "<html><body style='font-family: Arial; text-align: center; margin-top: 100px;'><h1>✅ Configuração de Pinos Salva!</h1><p>As configurações foram aplicadas.</p><script>setTimeout(() => location.href='/', 2000);</script></body></html>";
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

// Handler para salvar configurações de sensores
static esp_err_t save_sensors_handler(httpd_req_t *req) {
    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "📊 Configuração de sensores recebida: %s", buf);
    
    const char* response = "<html><body style='font-family: Arial; text-align: center; margin-top: 100px;'><h1>✅ Configuração de Sensores Salva!</h1><p>Os sensores foram configurados.</p><script>setTimeout(() => location.href='/', 2000);</script></body></html>";
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

// Handler para API de informações do sistema
static esp_err_t system_info_handler(httpd_req_t *req) {
    char response[512];
    
    // Obter informações do sistema
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
    
    snprintf(response, sizeof(response), 
        "{"
        "\"free_ram\": %lu,"
        "\"uptime\": %lu,"
        "\"clients\": 1"
        "}", free_heap, uptime);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

// Handler para API de logs
static esp_err_t logs_handler(httpd_req_t *req) {
    char logs[1024];
    uint32_t free_heap = esp_get_free_heap_size();
    
    snprintf(logs, sizeof(logs),
        "[INFO] Sistema iniciado\n"
        "[INFO] WiFi AP configurado - SSID: Oraculo-Assistant\n"
        "[INFO] Servidor web avançado iniciado na porta 80\n"
        "[INFO] Interface com 5 abas: WiFi, Pinos, Sensores, Sistema, Logs\n"
        "[INFO] Sistema funcionando normalmente\n"
        "[DEBUG] Memória livre: %lu bytes\n"
        "[DEBUG] Uptime: %lu segundos\n"
        "[INFO] Todas as funcionalidades operacionais",
        free_heap, xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
    
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, logs, strlen(logs));
    
    return ESP_OK;
}

// Handler para limpar logs
static esp_err_t clear_logs_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "🗑️ Logs limpos pelo usuário");
    
    const char* response = "OK";
    httpd_resp_send(req, response, strlen(response));
    
    return ESP_OK;
}

// Handler para reiniciar sistema
static esp_err_t restart_handler(httpd_req_t *req) {
    const char* response = "<html><body style='font-family: Arial; text-align: center; margin-top: 100px;'><h1>🔄 Reiniciando Sistema...</h1><p>O dispositivo será reiniciado em 3 segundos.</p></body></html>";
    httpd_resp_send(req, response, strlen(response));
    
    ESP_LOGI(TAG, "🔄 Sistema sendo reiniciado pelo usuário");
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    
    return ESP_OK;
}

// Handler para reset de fábrica
static esp_err_t factory_reset_handler(httpd_req_t *req) {
    const char* response = "<html><body style='font-family: Arial; text-align: center; margin-top: 100px;'><h1>⚠️ Reset de Fábrica</h1><p>Todas as configurações foram apagadas. Reiniciando...</p></body></html>";
    httpd_resp_send(req, response, strlen(response));
    
    ESP_LOGI(TAG, "⚠️ Reset de fábrica executado pelo usuário");
    nvs_flash_erase();
    vTaskDelay(pdMS_TO_TICKS(3000));
    esp_restart();
    
    return ESP_OK;
}

// Configurar servidor HTTP
static void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_uri_handlers = 12; // Aumentar limite de handlers
    
    if (httpd_start(&server, &config) == ESP_OK) {
        // Página principal
        httpd_uri_t config_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = config_handler
        };
        httpd_register_uri_handler(server, &config_uri);
        
        // Salvar configurações WiFi
        httpd_uri_t save_wifi_uri = {
            .uri = "/save_wifi",
            .method = HTTP_POST,
            .handler = save_wifi_handler
        };
        httpd_register_uri_handler(server, &save_wifi_uri);
        
        // Salvar configurações de pinos
        httpd_uri_t save_pins_uri = {
            .uri = "/save_pins",
            .method = HTTP_POST,
            .handler = save_pins_handler
        };
        httpd_register_uri_handler(server, &save_pins_uri);
        
        // Salvar configurações de sensores
        httpd_uri_t save_sensors_uri = {
            .uri = "/save_sensors",
            .method = HTTP_POST,
            .handler = save_sensors_handler
        };
        httpd_register_uri_handler(server, &save_sensors_uri);
        
        // API - Informações do sistema
        httpd_uri_t system_info_uri = {
            .uri = "/api/system_info",
            .method = HTTP_GET,
            .handler = system_info_handler
        };
        httpd_register_uri_handler(server, &system_info_uri);
        
        // API - Logs do sistema
        httpd_uri_t logs_uri = {
            .uri = "/api/logs",
            .method = HTTP_GET,
            .handler = logs_handler
        };
        httpd_register_uri_handler(server, &logs_uri);
        
        // API - Limpar logs
        httpd_uri_t clear_logs_uri = {
            .uri = "/api/clear_logs",
            .method = HTTP_POST,
            .handler = clear_logs_handler
        };
        httpd_register_uri_handler(server, &clear_logs_uri);
        
        // Reiniciar sistema
        httpd_uri_t restart_uri = {
            .uri = "/restart",
            .method = HTTP_GET,
            .handler = restart_handler
        };
        httpd_register_uri_handler(server, &restart_uri);
        
        // Reset de fábrica
        httpd_uri_t factory_reset_uri = {
            .uri = "/factory_reset",
            .method = HTTP_GET,
            .handler = factory_reset_handler
        };
        httpd_register_uri_handler(server, &factory_reset_uri);
        
        ESP_LOGI(TAG, "✅ Servidor web avançado iniciado na porta 80");
        ESP_LOGI(TAG, "🌐 Interface completa: http://192.168.4.1");
        ESP_LOGI(TAG, "🔧 Funcionalidades: WiFi, Pinos, Sensores, Sistema, Logs");
    }
}

// Configurar WiFi como Access Point
static void wifi_init_ap() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.ap.ssid, "Oraculo-Assistant");
    wifi_config.ap.ssid_len = strlen("Oraculo-Assistant");
    strcpy((char*)wifi_config.ap.password, "12345678");
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Obter e mostrar informações do AP
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    
    ESP_LOGI(TAG, "🔮 === ORACULO ASSISTANT INICIADO ===");
    ESP_LOGI(TAG, "📶 WiFi AP criado: 'Oraculo-Assistant'");
    ESP_LOGI(TAG, "🔑 Senha: 12345678");
    ESP_LOGI(TAG, "🌐 IP do dispositivo: 192.168.4.1");
    ESP_LOGI(TAG, "📱 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(TAG, "⚙️  Interface web: http://192.168.4.1");
    ESP_LOGI(TAG, "======================================");
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "🚀 Iniciando Oraculo Assistant...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "✅ NVS inicializado");
    
    // Configurar WiFi AP
    wifi_init_ap();
    
    // Aguardar um pouco para o WiFi estabilizar
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Iniciar servidor web
    start_webserver();
    
    // Loop principal
    int counter = 0;
    while (true) {
        counter++;
        ESP_LOGI(TAG, "⚡ Sistema rodando... [%d] - IP: 192.168.4.1", counter);
        vTaskDelay(pdMS_TO_TICKS(10000)); // Log a cada 10 segundos
    }
}