#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "crawler.h"
#include "curl.h"
#include "url.h"
#include "pagedir.h"
#include "pagedir.c"

// Function prototypes
static void parseArgs(const int argc, char *argv[], char **seedURL, char **pageDirectory, int *maxDepth);
static void crawl(char *seedURL, char *pageDirectory, const int maxDepth);
static void pageScan(webpage_t *page, bag_t *pagesToCrawl, hashtable_t *pagesSeen);



// Function to initialize the bag
bag_t *init_bag() {
    bag_t *bag = malloc(sizeof(bag_t));
    bag->count = 0;
    return bag;
}

// Function to initialize the hashtable
hashtable_t *init_hashtable() {
    hashtable_t *ht = malloc(sizeof(hashtable_t));
    ht->count = 0;
    return ht;
}
// Hash table structure
struct hashtable {
    char urls[MAX_URLS][500];
    int count;
};

// Bag structure
struct bag {
    webpage_t pages[MAX_URLS];
    int count;
};


// Function to check if a URL is in the hashtable
int in_hashtable(hashtable_t *ht, char *url) {
    for (int i = 0; i < ht->count; i++) {
        if (strcmp(ht->urls[i], url) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to add a webpage to the bag
void add_to_bag(bag_t *bag, webpage_t wp) {
    if (bag->count < MAX_URLS) {
//add the webpage to the next available position
        //printf("%d Added: %s\n", wp.depth, wp.url);
        bag->pages[bag->count] = wp;
//increment the count
        bag->count++;
    }
}

// Function to add a URL to the hashtable
bool add_to_hashtable(hashtable_t *ht, char *url) {
if (!in_hashtable(ht, url)) {
//check if hashtable is full
    if (ht->count < MAX_URLS) {
//add the url to the next available position in the hashtable array
        strcpy(ht->urls[ht->count], url);
//increment count of urls in the hashtbale
        ht->count++;
            return true;
    }
} else {
        return false;
    }
}

// Function to fetch webpage using curl
char *fetch_webpage(char *url, int depth) {
    //printf("%d Fetched: %s\n", depth, url);
    size_t size;
    char *page_content = download(url, &size);
    return page_content;
}

// Function to save webpage to a file
void save_webpage(char *pageDirectory, webpage_t wp, int documentID) {
    char filepath[512];
    static int file_count = 0;
    snprintf(filepath, sizeof(filepath), "%s/%d", pageDirectory, documentID);
    // printf("%d Scanning: %s\n", wp.depth, wp.url);


printf("%d: %s\n", wp.depth, wp.url);

    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
//this menas that u wont be able to open the file for writing
        fprintf(stderr, "Unable to open file: %s\n", filepath);
        return;
    }

    fputs(wp.html, file);
    fclose(file);
}

// Function to normalize URL
char *normalize_url(char *url, char *base_url) {
    char *normalized_url = malloc(500);

    size_t length = strlen(url);
    if (url[0] == '/') {
        url++;
    }
    if (strncmp(url, "http", 4) != 0) {
//if url does not start with http it is then considere d a relative url??
        if(url[0] == '/'){
        snprintf(normalized_url, 500, "%s%s", base_url, url);
        }else{
            snprintf(normalized_url, 500, "%s/%s", base_url, url);
        }
    } else {
//if the url starts with http it is an absolute url
        strncpy(normalized_url, url, 500);
    }
    return normalized_url;
}


static void parseArgs(const int argc, char *argv[], char **seedURL, char **pageDirectory, int *maxDepth) {
    if (argc != 4) {
        fprintf(stderr, "Usage: ./crawler seedURL pageDirectory maxDepth\n");
        exit(1);
    }

    *seedURL = argv[1];
    *pageDirectory = argv[2];
    *maxDepth = atoi(argv[3]);

    if (*maxDepth < 0 || *maxDepth > 10) {
        fprintf(stderr, "maxDepth should be between 0 and 10\n");
        exit(1);
    }
}
int is_valid_url(char *url) {
    return (strchr(url,'#') == NULL && url[0] != '\0');
//remove the hashes in the possible urls
}

bool is_mailto(const char *url){
return strncmp(url, "mailto:", 7)==0;
}

static void pageScan(webpage_t *page, bag_t *pagesToCrawl, hashtable_t *pagesSeen) {
    char *pos = page->html;
    while ((pos = strstr(pos, "<a href=\"")) != NULL) {
        char *url_start = pos + 9;
        char *url_end = strstr(url_start, "\"");
        if (url_end != NULL) {
            char url[500];
            strncpy(url, url_start, url_end -url_start);
            url[url_end - url_start] = '\0';
            char *normalized_url = normalize_url(url, page->url);
if (isInternalURL(page->url, normalized_url) && is_valid_url(normalized_url) && !is_mailto(normalized_url)) {
                if (!in_hashtable(pagesSeen, normalized_url)) {
                    add_to_hashtable(pagesSeen,normalized_url);
                    webpage_t new_page;
                    new_page.url = normalized_url;
                    new_page.depth = page->depth + 1;
                    add_to_bag(pagesToCrawl, new_page);
                }
            } else {
//TESTING PURPOUSE IGNORE
                //printf("%d IgnDupl: %s\n", page->depth, normalized_url);
            }
        }
        pos = url_end;
    }
}

static void crawl(char *seedURL, char *pageDirectory, const int maxDepth) {
    hashtable_t *pagesSeen = init_hashtable();
    bag_t *pagesToCrawl = init_bag();//creates a bag to store webpages to crawl
    add_to_hashtable(pagesSeen,seedURL);
    webpage_t seedPage;
    seedPage.url = seedURL;
    seedPage.depth = 0;
    add_to_bag(pagesToCrawl, seedPage);

int documentId = 1;
    while (pagesToCrawl->count > 0) {
        webpage_t current_page = pagesToCrawl->pages[pagesToCrawl->count -1];
//this will give the last webpage from the bag
        pagesToCrawl->count--;
        sleep(1); // pauses  for one second liekr reccomended in slack?
        char *html = fetch_webpage(current_page.url, current_page.depth);
        current_page.html = html;
        if (html != NULL) {
            save_webpage(pageDirectory, current_page, documentId);
//this will save the webpage into a file
            if (current_page.depth < maxDepth) {
                pageScan(&current_page,pagesToCrawl,pagesSeen);
//scan webpages for urls and then add those to the bag
            }
            documentId++;
        }
        free(html); // Free the memory allocated by fetch_webpage
    }
    free(pagesToCrawl);
    free(pagesSeen);
}

int main(const int argc, char *argv[]) {
    char *seedURL;
    int maxDepth;
    char *pageDirectory;
    parseArgs(argc, argv, &seedURL, &pageDirectory, &maxDepth);
    pagedir_init(pageDirectory);
    crawl(seedURL, pageDirectory, maxDepth);

    return 0;
}
