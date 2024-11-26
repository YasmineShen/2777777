#include "t27.h"

static int char_to_index(char c) {
    if (c == '\''){
    return ALPHA-1 ;
    } ; // Apostrophe
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    return -1; // Invalid character
}

dict* dict_init(void) {
    dict* node = (dict*)malloc(sizeof(dict));
    if (!node) return NULL;
    for (int i = 0; i < ALPHA; i++) {
        node->dwn[i] = NULL;
    }
    node->up = NULL;
    node->terminal = false;
    node->freq = 0;
    return node;
}

bool dict_addword(dict* p, const char* wd) {
    if (p == NULL || wd == NULL) return false;

    dict* current = p;
    for (const char* c = wd; *c; ++c) {
        int index = *c - 'a';
        if (*c == '\'') index = 26; // Special case for apostrophe

        if (current->dwn[index] == NULL) {
            // Allocate memory for the next node and initialize it
            current->dwn[index] = (dict*)malloc(sizeof(dict));
            memset(current->dwn[index], 0, sizeof(dict));
            current->dwn[index]->up = current; // Set parent pointer
        }
        current = current->dwn[index]; // Move to the next node
    }

    if (!current->terminal) { // If the word is not already in the dictionary
        current->terminal = true; // Mark this node as the end of a word
        current->freq = 1;        // Initialize frequency to 1
        return true;              // Indicate that a new word was added
    } else {
        current->freq++; // Increment frequency if the word already exists
        return false;    // Indicate that the word already existed
    }
}

int dict_nodecount(const dict* p) {
    if (!p) return 0;
    int count = 1; // Count current node
    for (int i = 0; i < ALPHA; i++) {
        count += dict_nodecount(p->dwn[i]);
    }
    return count;
}

int dict_wordcount(const dict* p) {
    if (!p) return 0;
    int count = p->terminal ? p->freq : 0;
    for (int i = 0; i < ALPHA; i++) {
        count += dict_wordcount(p->dwn[i]);
    }
    return count;
}

dict* dict_spell(const dict* p, const char* str) {
    if (!p || !str) return NULL;

    const dict* current = p;
    for (const char* c = str; *c != '\0'; c++) {
        int index = char_to_index(*c);
        if (index == -1 || !current->dwn[index]) return NULL;
        current = current->dwn[index];
    }

    return current->terminal ? (dict*)current : NULL;
}

void dict_free(dict** p) {
    if (!p || !*p) return;

    for (int i = 0; i < ALPHA; i++) {
        dict_free(&((*p)->dwn[i]));
    }
    free(*p);
    *p = NULL;
}

int dict_mostcommon(const dict* p) {
    if (!p) return 0;

    int max_freq = p->terminal ? p->freq : 0;
    for (int i = 0; i < ALPHA; i++) {
        int child_max = dict_mostcommon(p->dwn[i]);
        if (child_max > max_freq) {
            max_freq = child_max;
        }
    }
    return max_freq;
}

unsigned dict_cmp(dict* p1, dict* p2) {
    if (!p1 || !p2) return 0;

    // Step 1: Find the depth of both nodes
    unsigned depth1 = 0, depth2 = 0;
    dict* temp1 = p1;
    dict* temp2 = p2;

    while (temp1) {
        depth1++;
        temp1 = temp1->up;
    }

    while (temp2) {
        depth2++;
        temp2 = temp2->up;
    }

    // Step 2: Bring both nodes to the same depth
    unsigned distance = 0;
    while (depth1 > depth2) {
        p1 = p1->up;
        depth1--;
        distance++;
    }

    while (depth2 > depth1) {
        p2 = p2->up;
        depth2--;
        distance++;
    }

    // Step 3: Find the common ancestor
    while (p1 != p2) {
        p1 = p1->up;
        p2 = p2->up;
        distance += 2; // One step for each node
    }

    return distance;
}

// Helper function to perform depth-first search for autocomplete
void dfs(const dict* node, char* path, int depth, char* ret, int* max_freq) {
    if (!node) return;

    // If the current node is terminal, consider it as a candidate for autocomplete
    if (node->terminal) {
        if (node->freq > *max_freq || (node->freq == *max_freq && strcmp(path, ret) < 0)) {
            *max_freq = node->freq;
            path[depth] = '\0';
            strcpy(ret, path);
        }
    }

    // Recurse into all children nodes
    for (int i = 0; i < ALPHA; i++) {
        if (node->dwn[i]) {
            path[depth] = (i == 26) ? '\'' : 'a' + i;
            dfs(node->dwn[i], path, depth + 1, ret, max_freq);
        }
    }
}

void dict_autocomplete(const dict* p, const char* wd, char* ret) {
    if (!p || !wd || !ret) {
        ret[0] = '\0';
        return;
    }

    const dict* current = p;

    // Traverse to the node corresponding to the given prefix
    for (const char* c = wd; *c != '\0'; c++) {
        int index = char_to_index(*c);
        if (index == -1 || !current->dwn[index]) {
            ret[0] = '\0';
            return;
        }
        current = current->dwn[index];
    }

    // If the prefix itself is a complete word, return it directly
    if (current->terminal) {
        strcpy(ret, wd);
        return;
    }

    // Otherwise, perform a depth-first search to find the most frequent word from this point
    char buffer[100] = "";
    int max_freq = -1;
    dfs(current, buffer, 0, ret, &max_freq);

    // If no more frequent word is found, return the prefix itself
    if (max_freq == -1) {
        strcpy(ret, wd);
    }
}
