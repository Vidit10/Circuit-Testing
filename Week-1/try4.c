#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NAME_LEN 50
#define MAX_CONNECTIONS 10

typedef enum { INPUT, OUTPUT, WIRE, GATE } NodeType;

typedef struct CircuitNode {
    char name[MAX_NAME_LEN];
    NodeType type;
    struct CircuitNode* inputs[MAX_CONNECTIONS];
    struct CircuitNode* outputs[MAX_CONNECTIONS];
    int input_count;
    int output_count;
} CircuitNode;

CircuitNode* create_node(const char* name, NodeType type) {
    CircuitNode* node = (CircuitNode*)malloc(sizeof(CircuitNode));
    strncpy(node->name, name, MAX_NAME_LEN);
    node->type = type;
    node->input_count = 0;
    node->output_count = 0;
    return node;
}

void add_connection(CircuitNode* src, CircuitNode* dest) {
    if(src->output_count < MAX_CONNECTIONS && dest->input_count < MAX_CONNECTIONS) {
        src->outputs[src->output_count++] = dest;
        dest->inputs[dest->input_count++] = src;
    }
}

void print_connections(CircuitNode* node) {
    printf(" %s\n", node->name);
    printf("  %s\n", "|");
    
    // Print inputs
    if(node->input_count > 0) {
        printf("  |-[Inputs]-\n");
        for(int i = 0; i < node->input_count; i++) {
            printf("  |  %d: --> %s\n", i, node->inputs[i]->name);
        }
    }
    
    // Print outputs
    if(node->output_count > 0) {
        printf("  |-[Outputs]-\n");
        for(int i = 0; i < node->output_count; i++) {
            printf("  |  %d: %s -->\n", i, node->outputs[i]->name);
        }
    }
    printf("\n");
}

CircuitNode* parse_verilog(const char* filename) {
    FILE* file = fopen(filename, "r");
    if(!file) {
        perror("Error opening file");
        return NULL;
    }

    CircuitNode* head = NULL;
    CircuitNode* current = NULL;
    char line[256];

    while(fgets(line, sizeof(line), file)) {
        char* token = strtok(line, " \t\n");
        if(!token) continue;

        if(strcmp(token, "input") == 0) {
            char* name = strtok(NULL, " ,;\t\n");
            current = create_node(name, INPUT);
            if(!head) head = current;
        }
        else if(strcmp(token, "output") == 0) {
            char* name = strtok(NULL, " ,;\t\n");
            current = create_node(name, OUTPUT);
        }
        else if(strcmp(token, "wire") == 0) {
            char* name = strtok(NULL, " ,;\t\n");
            current = create_node(name, WIRE);
        }
        else if(strncmp(token, "nand", 4) == 0 || 
                strncmp(token, "and", 3) == 0 ||
                strncmp(token, "or", 2) == 0) {
            char gate_name[MAX_NAME_LEN];
            char output_name[MAX_NAME_LEN];
            char input1[MAX_NAME_LEN];
            char input2[MAX_NAME_LEN];
            
            sscanf(token, "%*[^(](%[^,], %[^,], %[^)])", output_name, input1, input2);
            
            // Create or find nodes
            CircuitNode* out_node = create_node(output_name, GATE);
            CircuitNode* in1_node = create_node(input1, WIRE);
            CircuitNode* in2_node = create_node(input2, WIRE);
            
            // Create connections
            add_connection(in1_node, out_node);
            add_connection(in2_node, out_node);
            
            if(!head) head = out_node;
        }
    }
    fclose(file);
    return head;
}

void visualize_circuit(CircuitNode* start) {
    printf("\nCircuit Visualization:\n");
    printf("======================\n");
    
    CircuitNode* current = start;
    while(current != NULL) {
        print_connections(current);
        
        // Simple traversal - goes to first output
        current = (current->output_count > 0) ? 
                 current->outputs[0] : NULL;
    }
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        printf("Usage: %s <verilog_file.v>\n", argv[0]);
        return 1;
    }
    
    CircuitNode* circuit = parse_verilog(argv[1]);
    if(!circuit) {
        printf("Failed to parse Verilog file\n");
        return 1;
    }
    
    visualize_circuit(circuit);
    return 0;
}
