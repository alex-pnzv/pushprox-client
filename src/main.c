#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>

#define MAX_URL_LENGTH 1024
#define INITIAL_RESPONSE_BUFFER_SIZE 2048
#define TIMEOUT 30
#define CONNECT_TIMEOUT 5


size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    char **buffer = userp;
    size_t current_size = strlen(*buffer);

    if (current_size + total_size + 1 > INITIAL_RESPONSE_BUFFER_SIZE) {
        size_t new_size = INITIAL_RESPONSE_BUFFER_SIZE * 2;
        while (new_size < current_size + total_size + 1) {
            new_size *= 2;
        }

        char *new_buffer = realloc(*buffer, new_size);
        if (!new_buffer) {
            fprintf(stderr, "Memory reallocation failed!\n");
            return 0;
        }
        *buffer = new_buffer;
    }

    // Append the received data to the buffer
    memcpy(*buffer + current_size, contents, total_size);
    (*buffer)[current_size + total_size] = '\0';

    return total_size;
}

char *allocate_response_buffer() {
    char *buffer = malloc(INITIAL_RESPONSE_BUFFER_SIZE);
    if (buffer) {
        buffer[0] = '\0';  // Initialize the buffer
    }
    return buffer;
}

int do_push(const char *proxy_url, const char *scrape_id, char *metrics) {
    CURL *curl;
    CURLcode res;
    char proxy_poll_url[MAX_URL_LENGTH];
    const char path[] = "/push";
    size_t metrics_size = strlen(metrics);
    size_t post_data_size = metrics_size + 1000;
    char headers[] =
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nId: %s\r\n\r\n%s";
    char *post_data = malloc(post_data_size);

    if (!post_data) {
        fprintf(stderr, "Memory allocation failed for post_data!\n");
        return 1;
    }

    snprintf(post_data, post_data_size, headers, scrape_id, metrics);
    //printf("%s\n", post_data);

    // Ensure the buffer is large enough to hold the full URL
    if (strlen(proxy_url) + strlen(path) >= MAX_URL_LENGTH) {
        fprintf(stderr, "URL length exceeds buffer size!\n");
        return 1;
    }

    // Build the full URL
    strcpy(proxy_poll_url, proxy_url);
    strcat(proxy_poll_url, path);

    char *response_buffer = allocate_response_buffer();
    if (!response_buffer) {
        fprintf(stderr, "Memory allocation failed!\n");
        free(post_data);
        return 1;
    }

    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Failed to initialize CURL!\n");
        free(post_data);
        free(response_buffer);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, proxy_poll_url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
        fprintf(stderr, "Push request failed: %s\n", curl_easy_strerror(res));
    }

    fprintf(stdout, "Push request done. scrape_id: %s, status: %s.\n",scrape_id, curl_easy_strerror(res));

    free(post_data);
    free(response_buffer);
    curl_easy_cleanup(curl);
    return 0;
}

int do_scrape(char *host, const char *proxy_url, char *scrape_id) {
    CURL *curl;
    CURLcode res;

    char *response_buffer = allocate_response_buffer();
    if (!response_buffer) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Failed to initialize CURL!\n");
        free(response_buffer);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, host);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
        fprintf(stderr, "Scrape request failed: %s\n", curl_easy_strerror(res));
    }

    do_push(proxy_url,scrape_id, response_buffer);
    //printf("Response: %s\n", response_buffer);

    free(response_buffer);
    curl_easy_cleanup(curl);
    return 0;
}

int do_poll(const char *proxy_url, const char *post_data){
    CURL *curl;
    CURLcode res;
    char exporter_url[100];
    char scrape_id [64];
    char proxy_poll_url[MAX_URL_LENGTH];
    char path[] = "/poll";

    // Ensure the buffer is large enough to hold the full URL
    if (strlen(proxy_url) + strlen(path) >= MAX_URL_LENGTH) {
        fprintf(stderr, "URL length exceeds buffer size\n");
        return 1;
    }

    // Build the full URL
    strcpy(proxy_poll_url, proxy_url);
    strcat(proxy_poll_url, path);

    char *response_buffer = allocate_response_buffer();

    if (!response_buffer) {
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    curl = curl_easy_init();
    if(!curl) {
        fprintf(stderr, "Failed to initialize CURL!\n");
        free(response_buffer);
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, proxy_poll_url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
        fprintf(stderr, "Poll request failed: %s\n", curl_easy_strerror(res));
        free(response_buffer);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Example of response:
    //      GET http://192.168.1.159:9182/metrics HTTP/1.1
    //      Host: 192.168.1.159:9182
    //      User-Agent: Prometheus/2.54.0
    //      Accept: application/openmetrics-text;version=1.0.0;q=0.5,application/openmetrics-text;version=0.0.1;q=0.4,text/plain;ver
    //      sion=0.0.4;q=0.3,*/*;q=0.2
    //      Accept-Encoding: gzip
    //      Id: 508c3ca5-5ab2-4c81-b5e6-3f926be73f65
    //      X-Prometheus-Scrape-Timeout-Seconds: 10

    sscanf(response_buffer, "GET %99s HTTP/1.1", exporter_url);

    char *headers_start = strstr(response_buffer, "\r\n") + 2; // Skip past the request line
    char *line = strtok(headers_start, "\r\n");
    while (line != NULL) {
        if (strncmp(line, "Id: ", 4) == 0) {
            // Extract the value after "Id: "
            sscanf(line + 4, "%1023s", scrape_id);
            break; // Assuming only one "Id" header
        }
        // Move to the next line
        line = strtok(NULL, "\r\n");
    }

    //printf("Exporter URL: %s, scrape_id: %s\n", exporter_url, scrape_id);
    do_scrape(exporter_url, proxy_url,scrape_id);

    free(response_buffer);
    curl_easy_cleanup(curl);
    return 0;
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <URL> <IP>\n", argv[0]);
        return 1;
    }

    const char *url = argv[1];
    const char *ip = argv[2];

    while (1) {
        curl_global_init(CURL_GLOBAL_ALL);
        do_poll(url, ip);
        curl_global_cleanup();
        sleep(1);
    }

    return 0;
}

