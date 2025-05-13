#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Gate types
typedef enum {
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    NOT,
    BUFF,
    INPUT,
    OUTPUT,
    BRANCH
} GateType;

// Node structure for the linked list
typedef struct Node {
    int id;                 // Node identifier
    char name[50];          // Node name
    GateType type;          // Type of gate
    int numInputs;          // Number of inputs to this gate
    struct Node **inputs;   // Array of pointers to input nodes
    int numOutputs;         // Number of outputs from this node
    struct Node **outputs;  // Array of pointers to output nodes
    bool value;             // Current value of the node
    bool visited;           // Flag for traversal algorithms
    int pathCount;          // For path counting algorithm
} Node;

// Circuit structure
typedef struct {
    Node **nodes;           // Array of all nodes
    int nodeCount;          // Total number of nodes
    Node **inputs;          // Array of input nodes
    int inputCount;         // Number of input nodes
    Node **outputs;         // Array of output nodes
    int outputCount;        // Number of output nodes
} Circuit;

// Gate functions from functional_codes.c
bool andFunction(bool gate1, bool gate2) {
    if (gate1 == 1 && gate2 == 1) {
        return 1;
    }
    return 0;
}

bool orFunction(bool gate1, bool gate2) {
    if (gate1 == 0 && gate2 == 0) {
        return 0;
    }
    return 1;
}

bool notFunction(bool gate) {
    return !gate;
}

bool nandFunction(bool gate1, bool gate2) {
    return notFunction(andFunction(gate1, gate2));
}

bool norFunction(bool gate1, bool gate2) {
    return notFunction(orFunction(gate1, gate2));
}

bool xorFunction(bool gate1, bool gate2) {
    if (gate1 == gate2) {
        return 0;
    }
    return 1;
}

bool xnorFunction(bool gate1, bool gate2) {
    return notFunction(xorFunction(gate1, gate2));
}

bool buffFunction(bool gate) {
    return gate;
}

// Function to create a new node
Node* createNode(int id, const char* name, GateType type) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    newNode->id = id;
    strncpy(newNode->name, name, 49);
    newNode->name[49] = '\0';
    newNode->type = type;
    newNode->numInputs = 0;
    newNode->inputs = NULL;
    newNode->numOutputs = 0;
    newNode->outputs = NULL;
    newNode->value = false;
    newNode->visited = false;
    newNode->pathCount = 0;
    
    return newNode;
}

// Function to add an input to a node
void addInput(Node* node, Node* input) {
    node->numInputs++;
    node->inputs = (Node**)realloc(node->inputs, node->numInputs * sizeof(Node*));
    if (!node->inputs) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    node->inputs[node->numInputs - 1] = input;
    
    // Also add this node as an output to the input node
    input->numOutputs++;
    input->outputs = (Node**)realloc(input->outputs, input->numOutputs * sizeof(Node*));
    if (!input->outputs) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    input->outputs[input->numOutputs - 1] = node;
}

// Function to initialize a circuit
Circuit* initCircuit() {
    Circuit* circuit = (Circuit*)malloc(sizeof(Circuit));
    if (!circuit) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    circuit->nodes = NULL;
    circuit->nodeCount = 0;
    circuit->inputs = NULL;
    circuit->inputCount = 0;
    circuit->outputs = NULL;
    circuit->outputCount = 0;
    
    return circuit;
}

// Function to add a node to the circuit
void addNode(Circuit* circuit, Node* node) {
    circuit->nodeCount++;
    circuit->nodes = (Node**)realloc(circuit->nodes, circuit->nodeCount * sizeof(Node*));
    if (!circuit->nodes) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    circuit->nodes[circuit->nodeCount - 1] = node;
    
    // If it's an input node, add it to the inputs array
    if (node->type == INPUT) {
        circuit->inputCount++;
        circuit->inputs = (Node**)realloc(circuit->inputs, circuit->inputCount * sizeof(Node*));
        if (!circuit->inputs) {
            printf("Memory allocation failed\n");
            exit(1);
        }
        circuit->inputs[circuit->inputCount - 1] = node;
    }
    
    // If it's an output node, add it to the outputs array
    if (node->type == OUTPUT) {
        circuit->outputCount++;
        circuit->outputs = (Node**)realloc(circuit->outputs, circuit->outputCount * sizeof(Node*));
        if (!circuit->outputs) {
            printf("Memory allocation failed\n");
            exit(1);
        }
        circuit->outputs[circuit->outputCount - 1] = node;
    }
}

// Function to find a node by name
Node* findNode(Circuit* circuit, const char* name) {
    for (int i = 0; i < circuit->nodeCount; i++) {
        if (strcmp(circuit->nodes[i]->name, name) == 0) {
            return circuit->nodes[i];
        }
    }
    return NULL;
}

// Function to parse the Verilog file and build the circuit
Circuit* parseVerilogFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        exit(1);
    }
    
    Circuit* circuit = initCircuit();
    char line[256];
    char name[50], type[50], input1[50], input2[50];
    
    // Skip comments and module declaration
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "input") || strstr(line, "output") || strstr(line, "wire")) {
            break;
        }
    }
    
    // Parse input declarations
    if (strstr(line, "input")) {
        char* token = strtok(line + 6, ","); // Skip "input "
        while (token != NULL) {
            // Remove any trailing semicolons or whitespace
            char* semicolon = strchr(token, ';');
            if (semicolon) *semicolon = '\0';
            
            // Trim whitespace
            while (*token == ' ' || *token == '\t' || *token == '\n') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
            
            if (strlen(token) > 0) {
                Node* node = createNode(circuit->nodeCount, token, INPUT);
                addNode(circuit, node);
            }
            
            token = strtok(NULL, ",");
        }
        
        // Get the next line
        fgets(line, sizeof(line), file);
    }
    
    // Parse output declarations
    if (strstr(line, "output")) {
        char* token = strtok(line + 7, ","); // Skip "output "
        while (token != NULL) {
            // Remove any trailing semicolons or whitespace
            char* semicolon = strchr(token, ';');
            if (semicolon) *semicolon = '\0';
            
            // Trim whitespace
            while (*token == ' ' || *token == '\t' || *token == '\n') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
            
            if (strlen(token) > 0) {
                Node* node = createNode(circuit->nodeCount, token, OUTPUT);
                addNode(circuit, node);
            }
            
            token = strtok(NULL, ",");
        }
        
        // Get the next line
        fgets(line, sizeof(line), file);
    }
    
    // Parse wire declarations
    if (strstr(line, "wire")) {
        char* token = strtok(line + 5, ","); // Skip "wire "
        while (token != NULL) {
            // Remove any trailing semicolons or whitespace
            char* semicolon = strchr(token, ';');
            if (semicolon) *semicolon = '\0';
            
            // Trim whitespace
            while (*token == ' ' || *token == '\t' || *token == '\n') token++;
            char* end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
            
            if (strlen(token) > 0) {
                // Create wire nodes (these will be connected later)
                Node* node = createNode(circuit->nodeCount, token, BRANCH);
                addNode(circuit, node);
            }
            
            token = strtok(NULL, ",");
        }
    }
    
    // Parse gate declarations
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "endmodule")) {
            break;
        }
        
        // Skip empty lines and comments
        if (strlen(line) <= 1 || line[0] == '/' || line[0] == '\n') {
            continue;
        }
        
        // Parse the gate declaration
        // Format: nand NAND2_1 (N10, N1, N3);
        if (sscanf(line, "%s %s (%[^,], %[^,], %[^)]);", type, name, name, input1, input2) == 5) {
            // Extract the output node name (first parameter)
            char output[50];
            sscanf(line, "%*s %*s (%[^,],", output);
            
            // Find or create the output node
            Node* outputNode = findNode(circuit, output);
            if (!outputNode) {
                outputNode = createNode(circuit->nodeCount, output, BRANCH);
                addNode(circuit, outputNode);
            }
            
            // Determine the gate type
            GateType gateType;
            if (strcmp(type, "nand") == 0) {
                gateType = NAND;
            } else if (strcmp(type, "and") == 0) {
                gateType = AND;
            } else if (strcmp(type, "or") == 0) {
                gateType = OR;
            } else if (strcmp(type, "nor") == 0) {
                gateType = NOR;
            } else if (strcmp(type, "xor") == 0) {
                gateType = XOR;
            } else if (strcmp(type, "xnor") == 0) {
                gateType = XNOR;
            } else if (strcmp(type, "not") == 0) {
                gateType = NOT;
            } else if (strcmp(type, "buff") == 0) {
                gateType = BUFF;
            } else {
                printf("Unknown gate type: %s\n", type);
                continue;
            }
            
            // Update the output node's type
            outputNode->type = gateType;
            
            // Find or create the input nodes and connect them
            Node* inputNode1 = findNode(circuit, input1);
            if (!inputNode1) {
                inputNode1 = createNode(circuit->nodeCount, input1, BRANCH);
                addNode(circuit, inputNode1);
            }
            
            Node* inputNode2 = findNode(circuit, input2);
            if (!inputNode2) {
                inputNode2 = createNode(circuit->nodeCount, input2, BRANCH);
                addNode(circuit, inputNode2);
            }
            
            // Connect the inputs to the output
            addInput(outputNode, inputNode1);
            addInput(outputNode, inputNode2);
        }
    }
    
    fclose(file);
    return circuit;
}

// Function to evaluate a node based on its inputs and gate type
bool evaluateNode(Node* node) {
    if (node->type == INPUT) {
        return node->value; // Input values are set externally
    }
    
    if (node->numInputs == 0) {
        return false; // No inputs, default to false
    }
    
    bool result = false;
    
    switch (node->type) {
        case AND:
            result = andFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case NAND:
            result = nandFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case OR:
            result = orFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case NOR:
            result = norFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case XOR:
            result = xorFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case XNOR:
            result = xnorFunction(node->inputs[0]->value, node->inputs[1]->value);
            break;
        case NOT:
            result = notFunction(node->inputs[0]->value);
            break;
        case BUFF:
            result = buffFunction(node->inputs[0]->value);
            break;
        case BRANCH:
            result = node->inputs[0]->value; // Pass through the input value
            break;
        case OUTPUT:
            if (node->numInputs > 0) {
                result = node->inputs[0]->value; // Pass through the input value
            }
            break;
        default:
            printf("Unknown gate type for node %s\n", node->name);
            break;
    }
    
    return result;
}

// Function to simulate the circuit
void simulateCircuit(Circuit* circuit, bool* inputValues) {
    // Set input values
    for (int i = 0; i < circuit->inputCount; i++) {
        circuit->inputs[i]->value = inputValues[i];
    }
    
    // Reset visited flags
    for (int i = 0; i < circuit->nodeCount; i++) {
        circuit->nodes[i]->visited = false;
    }
    
    // Topological evaluation
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < circuit->nodeCount; i++) {
            Node* node = circuit->nodes[i];
            if (node->type == INPUT) {
                continue; // Skip input nodes, they're already set
            }
            
            bool allInputsVisited = true;
            for (int j = 0; j < node->numInputs; j++) {
                if (!node->inputs[j]->visited) {
                    allInputsVisited = false;
                    break;
                }
            }
            
            if (allInputsVisited && !node->visited) {
                bool newValue = evaluateNode(node);
                if (newValue != node->value) {
                    node->value = newValue;
                    changed = true;
                }
                node->visited = true;
            }
        }
    }
}

// Function to count paths in the circuit
int countPaths(Circuit* circuit) {
    // Initialize path counts
    for (int i = 0; i < circuit->nodeCount; i++) {
        circuit->nodes[i]->pathCount = 0;
        circuit->nodes[i]->visited = false;
    }
    
    // Set path count for input nodes to 1
    for (int i = 0; i < circuit->inputCount; i++) {
        circuit->inputs[i]->pathCount = 1;
        circuit->inputs[i]->visited = true;
    }
    
    // Topological traversal for path counting
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < circuit->nodeCount; i++) {
            Node* node = circuit->nodes[i];
            if (node->visited) {
                continue;
            }
            
            bool allInputsVisited = true;
            for (int j = 0; j < node->numInputs; j++) {
                if (!node->inputs[j]->visited) {
                    allInputsVisited = false;
                    break;
                }
            }
            
            if (allInputsVisited) {
                // Calculate path count as sum of input path counts
                for (int j = 0; j < node->numInputs; j++) {
                    node->pathCount += node->inputs[j]->pathCount;
                }
                node->visited = true;
                changed = true;
            }
        }
    }
    
    // Sum path counts for output nodes
    int totalPaths = 0;
    for (int i = 0; i < circuit->outputCount; i++) {
        totalPaths += circuit->outputs[i]->pathCount;
    }
    
    return totalPaths;
}

// Function to free the circuit memory
void freeCircuit(Circuit* circuit) {
    if (!circuit) return;
    
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        if (node) {
            if (node->inputs) free(node->inputs);
            if (node->outputs) free(node->outputs);
            free(node);
        }
    }
    
    if (circuit->nodes) free(circuit->nodes);
    if (circuit->inputs) free(circuit->inputs);
    if (circuit->outputs) free(circuit->outputs);
    free(circuit);
}

// Function to save the circuit as a C file
void saveCircuitAsC(Circuit* circuit, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }
    
    // Write the header
    fprintf(file, "#include <stdio.h>\n");
    fprintf(file, "#include <stdlib.h>\n");
    fprintf(file, "#include <stdbool.h>\n");
    fprintf(file, "#include <string.h>\n\n");
    
    // Write the gate type enum
    fprintf(file, "// Gate types\n");
    fprintf(file, "typedef enum {\n");
    fprintf(file, "    AND,\n");
    fprintf(file, "    NAND,\n");
    fprintf(file, "    OR,\n");
    fprintf(file, "    NOR,\n");
    fprintf(file, "    XOR,\n");
    fprintf(file, "    XNOR,\n");
    fprintf(file, "    NOT,\n");
    fprintf(file, "    BUFF,\n");
    fprintf(file, "    INPUT,\n");
    fprintf(file, "    OUTPUT,\n");
    fprintf(file, "    BRANCH\n");
    fprintf(file, "} GateType;\n\n");
    
    // Write the node structure
    fprintf(file, "// Node structure for the linked list\n");
    fprintf(file, "typedef struct Node {\n");
    fprintf(file, "    int id;                 // Node identifier\n");
    fprintf(file, "    char name[50];          // Node name\n");
    fprintf(file, "    GateType type;          // Type of gate\n");
    fprintf(file, "    int numInputs;          // Number of inputs to this gate\n");
    fprintf(file, "    struct Node **inputs;   // Array of pointers to input nodes\n");
    fprintf(file, "    int numOutputs;         // Number of outputs from this node\n");
    fprintf(file, "    struct Node **outputs;  // Array of pointers to output nodes\n");
    fprintf(file, "    bool value;             // Current value of the node\n");
    fprintf(file, "    bool visited;           // Flag for traversal algorithms\n");
    fprintf(file, "    int pathCount;          // For path counting algorithm\n");
    fprintf(file, "} Node;\n\n");
    
    // Write the circuit structure
    fprintf(file, "// Circuit structure\n");
    fprintf(file, "typedef struct {\n");
    fprintf(file, "    Node **nodes;           // Array of all nodes\n");
    fprintf(file, "    int nodeCount;          // Total number of nodes\n");
    fprintf(file, "    Node **inputs;          // Array of input nodes\n");
    fprintf(file, "    int inputCount;         // Number of input nodes\n");
    fprintf(file, "    Node **outputs;         // Array of output nodes\n");
    fprintf(file, "    int outputCount;        // Number of output nodes\n");
    fprintf(file, "} Circuit;\n\n");
    
    // Write the gate functions
    fprintf(file, "// Gate functions\n");
    fprintf(file, "bool andFunction(bool gate1, bool gate2) {\n");
    fprintf(file, "    if (gate1 == 1 && gate2 == 1) {\n");
    fprintf(file, "        return 1;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    return 0;\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool orFunction(bool gate1, bool gate2) {\n");
    fprintf(file, "    if (gate1 == 0 && gate2 == 0) {\n");
    fprintf(file, "        return 0;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    return 1;\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool notFunction(bool gate) {\n");
    fprintf(file, "    return !gate;\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool nandFunction(bool gate1, bool gate2) {\n");
    fprintf(file, "    return notFunction(andFunction(gate1, gate2));\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool norFunction(bool gate1, bool gate2) {\n");
    fprintf(file, "    return notFunction(orFunction(gate1, gate2));\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool xorFunction(bool gate1, bool gate2) {\n");
    fprintf(file, "    if (gate1 == gate2) {\n");
    fprintf(file, "        return 0;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    return 1;\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool xnorFunction(bool gate1, bool gate2) {\n");
    fprintf(file, "    return notFunction(xorFunction(gate1, gate2));\n");
    fprintf(file, "}\n\n");
    
    fprintf(file, "bool buffFunction(bool gate) {\n");
    fprintf(file, "    return gate;\n");
    fprintf(file, "}\n\n");
    
    // Write the utility functions
    fprintf(file, "// Function to create a new node\n");
    fprintf(file, "Node* createNode(int id, const char* name, GateType type) {\n");
    fprintf(file, "    Node* newNode = (Node*)malloc(sizeof(Node));\n");
    fprintf(file, "    if (!newNode) {\n");
    fprintf(file, "        printf(\"Memory allocation failed\\n\");\n");
    fprintf(file, "        exit(1);\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    newNode->id = id;\n");
    fprintf(file, "    strncpy(newNode->name, name, 49);\n");
    fprintf(file, "    newNode->name[49] = '\\0';\n");
    fprintf(file, "    newNode->type = type;\n");
    fprintf(file, "    newNode->numInputs = 0;\n");
    fprintf(file, "    newNode->inputs = NULL;\n");
    fprintf(file, "    newNode->numOutputs = 0;\n");
    fprintf(file, "    newNode->outputs = NULL;\n");
    fprintf(file, "    newNode->value = false;\n");
    fprintf(file, "    newNode->visited = false;\n");
    fprintf(file, "    newNode->pathCount = 0;\n");
    fprintf(file, "    \n");
    fprintf(file, "    return newNode;\n");
    fprintf(file, "}\n\n");
    
    // Write the circuit initialization code
    fprintf(file, "// Function to initialize the circuit for c17\n");
    fprintf(file, "Circuit* initC17Circuit() {\n");
    fprintf(file, "    Circuit* circuit = (Circuit*)malloc(sizeof(Circuit));\n");
    fprintf(file, "    if (!circuit) {\n");
    fprintf(file, "        printf(\"Memory allocation failed\\n\");\n");
    fprintf(file, "        exit(1);\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    circuit->nodes = NULL;\n");
    fprintf(file, "    circuit->nodeCount = 0;\n");
    fprintf(file, "    circuit->inputs = NULL;\n");
    fprintf(file, "    circuit->inputCount = 0;\n");
    fprintf(file, "    circuit->outputs = NULL;\n");
    fprintf(file, "    circuit->outputCount = 0;\n\n");
    
    // Create nodes for the circuit
    fprintf(file, "    // Create nodes\n");
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        fprintf(file, "    Node* %s = createNode(%d, \"%s\", %s);\n", 
                node->name, node->id, node->name, 
                node->type == INPUT ? "INPUT" : 
                node->type == OUTPUT ? "OUTPUT" : 
                node->type == AND ? "AND" : 
                node->type == NAND ? "NAND" : 
                node->type == OR ? "OR" : 
                node->type == NOR ? "NOR" : 
                node->type == XOR ? "XOR" : 
                node->type == XNOR ? "XNOR" : 
                node->type == NOT ? "NOT" : 
                node->type == BUFF ? "BUFF" : "BRANCH");
    }
    
    fprintf(file, "\n    // Add nodes to circuit\n");
    for (int i = 0; i < circuit->nodeCount; i++) {
        fprintf(file, "    circuit->nodeCount++;\n");
        fprintf(file, "    circuit->nodes = (Node**)realloc(circuit->nodes, circuit->nodeCount * sizeof(Node*));\n");
        fprintf(file, "    circuit->nodes[circuit->nodeCount - 1] = %s;\n\n", circuit->nodes[i]->name);
    }
    
    fprintf(file, "    // Set up inputs array\n");
    for (int i = 0; i < circuit->inputCount; i++) {
        fprintf(file, "    circuit->inputCount++;\n");
        fprintf(file, "    circuit->inputs = (Node**)realloc(circuit->inputs, circuit->inputCount * sizeof(Node*));\n");
        fprintf(file, "    circuit->inputs[circuit->inputCount - 1] = %s;\n\n", circuit->inputs[i]->name);
    }
    
    fprintf(file, "    // Set up outputs array\n");
    for (int i = 0; i < circuit->outputCount; i++) {
        fprintf(file, "    circuit->outputCount++;\n");
        fprintf(file, "    circuit->outputs = (Node**)realloc(circuit->outputs, circuit->outputCount * sizeof(Node*));\n");
        fprintf(file, "    circuit->outputs[circuit->outputCount - 1] = %s;\n\n", circuit->outputs[i]->name);
    }
    
    fprintf(file, "    // Connect nodes\n");
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        for (int j = 0; j < node->numInputs; j++) {
            fprintf(file, "    %s->numInputs++;\n", node->name);
            fprintf(file, "    %s->inputs = (Node**)realloc(%s->inputs, %s->numInputs * sizeof(Node*));\n", 
                    node->name, node->name, node->name);
            fprintf(file, "    %s->inputs[%s->numInputs - 1] = %s;\n\n", 
                    node->name, node->name, node->inputs[j]->name);
            
            fprintf(file, "    %s->numOutputs++;\n", node->inputs[j]->name);
            fprintf(file, "    %s->outputs = (Node**)realloc(%s->outputs, %s->numOutputs * sizeof(Node*));\n", 
                    node->inputs[j]->name, node->inputs[j]->name, node->inputs[j]->name);
            fprintf(file, "    %s->outputs[%s->numOutputs - 1] = %s;\n\n", 
                    node->inputs[j]->name, node->inputs[j]->name, node->name);
        }
    }
    
    fprintf(file, "    return circuit;\n");
    fprintf(file, "}\n\n");
    
    // Write the evaluation function
    fprintf(file, "// Function to evaluate a node based on its inputs and gate type\n");
    fprintf(file, "bool evaluateNode(Node* node) {\n");
    fprintf(file, "    if (node->type == INPUT) {\n");
    fprintf(file, "        return node->value; // Input values are set externally\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    if (node->numInputs == 0) {\n");
    fprintf(file, "        return false; // No inputs, default to false\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    bool result = false;\n");
    fprintf(file, "    \n");
    fprintf(file, "    switch (node->type) {\n");
    fprintf(file, "        case AND:\n");
    fprintf(file, "            result = andFunction(node->inputs[0]->value, node->inputs[1]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case NAND:\n");
    fprintf(file, "            result = nandFunction(node->inputs[0]->value, node->inputs[1]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case OR:\n");
    fprintf(file, "            result = orFunction(node->inputs[0]->value, node->inputs[1]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case NOR:\n");
    fprintf(file, "            result = norFunction(node->inputs[0]->value, node->inputs[1]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case XOR:\n");
    fprintf(file, "            result = xorFunction(node->inputs[0]->value, node->inputs[1]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case XNOR:\n");
    fprintf(file, "            result = xnorFunction(node->inputs[0]->value, node->inputs[1]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case NOT:\n");
    fprintf(file, "            result = notFunction(node->inputs[0]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case BUFF:\n");
    fprintf(file, "            result = buffFunction(node->inputs[0]->value);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case BRANCH:\n");
    fprintf(file, "            result = node->inputs[0]->value; // Pass through the input value\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        case OUTPUT:\n");
    fprintf(file, "            if (node->numInputs > 0) {\n");
    fprintf(file, "                result = node->inputs[0]->value; // Pass through the input value\n");
    fprintf(file, "            }\n");
    fprintf(file, "            break;\n");
    fprintf(file, "        default:\n");
    fprintf(file, "            printf(\"Unknown gate type for node %%s\\n\", node->name);\n");
    fprintf(file, "            break;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    return result;\n");
    fprintf(file, "}\n\n");
    
    // Write the simulation function
    fprintf(file, "// Function to simulate the circuit\n");
    fprintf(file, "void simulateCircuit(Circuit* circuit, bool* inputValues) {\n");
    fprintf(file, "    // Set input values\n");
    fprintf(file, "    for (int i = 0; i < circuit->inputCount; i++) {\n");
    fprintf(file, "        circuit->inputs[i]->value = inputValues[i];\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Reset visited flags\n");
    fprintf(file, "    for (int i = 0; i < circuit->nodeCount; i++) {\n");
    fprintf(file, "        circuit->nodes[i]->visited = false;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Topological evaluation\n");
    fprintf(file, "    bool changed = true;\n");
    fprintf(file, "    while (changed) {\n");
    fprintf(file, "        changed = false;\n");
    fprintf(file, "        for (int i = 0; i < circuit->nodeCount; i++) {\n");
    fprintf(file, "            Node* node = circuit->nodes[i];\n");
    fprintf(file, "            if (node->type == INPUT) {\n");
    fprintf(file, "                continue; // Skip input nodes, they're already set\n");
    fprintf(file, "            }\n");
    fprintf(file, "            \n");
    fprintf(file, "            bool allInputsVisited = true;\n");
    fprintf(file, "            for (int j = 0; j < node->numInputs; j++) {\n");
    fprintf(file, "                if (!node->inputs[j]->visited) {\n");
    fprintf(file, "                    allInputsVisited = false;\n");
    fprintf(file, "                    break;\n");
    fprintf(file, "                }\n");
    fprintf(file, "            }\n");
    fprintf(file, "            \n");
    fprintf(file, "            if (allInputsVisited && !node->visited) {\n");
    fprintf(file, "                bool newValue = evaluateNode(node);\n");
    fprintf(file, "                if (newValue != node->value) {\n");
    fprintf(file, "                    node->value = newValue;\n");
    fprintf(file, "                    changed = true;\n");
    fprintf(file, "                }\n");
    fprintf(file, "                node->visited = true;\n");
    fprintf(file, "            }\n");
    fprintf(file, "        }\n");
    fprintf(file, "    }\n");
    fprintf(file, "}\n\n");
    
    // Write the path counting function
    fprintf(file, "// Function to count paths in the circuit\n");
    fprintf(file, "int countPaths(Circuit* circuit) {\n");
    fprintf(file, "    // Initialize path counts\n");
    fprintf(file, "    for (int i = 0; i < circuit->nodeCount; i++) {\n");
    fprintf(file, "        circuit->nodes[i]->pathCount = 0;\n");
    fprintf(file, "        circuit->nodes[i]->visited = false;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Set path count for input nodes to 1\n");
    fprintf(file, "    for (int i = 0; i < circuit->inputCount; i++) {\n");
    fprintf(file, "        circuit->inputs[i]->pathCount = 1;\n");
    fprintf(file, "        circuit->inputs[i]->visited = true;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Topological traversal for path counting\n");
    fprintf(file, "    bool changed = true;\n");
    fprintf(file, "    while (changed) {\n");
    fprintf(file, "        changed = false;\n");
    fprintf(file, "        for (int i = 0; i < circuit->nodeCount; i++) {\n");
    fprintf(file, "            Node* node = circuit->nodes[i];\n");
    fprintf(file, "            if (node->visited) {\n");
    fprintf(file, "                continue;\n");
    fprintf(file, "            }\n");
    fprintf(file, "            \n");
    fprintf(file, "            bool allInputsVisited = true;\n");
    fprintf(file, "            for (int j = 0; j < node->numInputs; j++) {\n");
    fprintf(file, "                if (!node->inputs[j]->visited) {\n");
    fprintf(file, "                    allInputsVisited = false;\n");
    fprintf(file, "                    break;\n");
    fprintf(file, "                }\n");
    fprintf(file, "            }\n");
    fprintf(file, "            \n");
    fprintf(file, "            if (allInputsVisited) {\n");
    fprintf(file, "                // Calculate path count as sum of input path counts\n");
    fprintf(file, "                for (int j = 0; j < node->numInputs; j++) {\n");
    fprintf(file, "                    node->pathCount += node->inputs[j]->pathCount;\n");
    fprintf(file, "                }\n");
    fprintf(file, "                node->visited = true;\n");
    fprintf(file, "                changed = true;\n");
    fprintf(file, "            }\n");
    fprintf(file, "        }\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Sum path counts for output nodes\n");
    fprintf(file, "    int totalPaths = 0;\n");
    fprintf(file, "    for (int i = 0; i < circuit->outputCount; i++) {\n");
    fprintf(file, "        totalPaths += circuit->outputs[i]->pathCount;\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    return totalPaths;\n");
    fprintf(file, "}\n\n");
    
    // Write the memory freeing function
    fprintf(file, "// Function to free the circuit memory\n");
    fprintf(file, "void freeCircuit(Circuit* circuit) {\n");
    fprintf(file, "    if (!circuit) return;\n");
    fprintf(file, "    \n");
    fprintf(file, "    for (int i = 0; i < circuit->nodeCount; i++) {\n");
    fprintf(file, "        Node* node = circuit->nodes[i];\n");
    fprintf(file, "        if (node) {\n");
    fprintf(file, "            if (node->inputs) free(node->inputs);\n");
    fprintf(file, "            if (node->outputs) free(node->outputs);\n");
    fprintf(file, "            free(node);\n");
    fprintf(file, "        }\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    if (circuit->nodes) free(circuit->nodes);\n");
    fprintf(file, "    if (circuit->inputs) free(circuit->inputs);\n");
    fprintf(file, "    if (circuit->outputs) free(circuit->outputs);\n");
    fprintf(file, "    free(circuit);\n");
    fprintf(file, "}\n\n");
    
    // Write the main function
    fprintf(file, "// Main function\n");
    fprintf(file, "int main() {\n");
    fprintf(file, "    // Initialize the c17 circuit\n");
    fprintf(file, "    Circuit* circuit = initC17Circuit();\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Count paths in the circuit\n");
    fprintf(file, "    int pathCount = countPaths(circuit);\n");
    fprintf(file, "    printf(\"Total number of paths in the circuit: %%d\\n\", pathCount);\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Simulate the circuit with all inputs set to 1\n");
    fprintf(file, "    bool inputValues[5] = {1, 1, 1, 1, 1};\n");
    fprintf(file, "    simulateCircuit(circuit, inputValues);\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Print output values\n");
    fprintf(file, "    printf(\"Output values:\\n\");\n");
    fprintf(file, "    for (int i = 0; i < circuit->outputCount; i++) {\n");
    fprintf(file, "        printf(\"%%s: %%d\\n\", circuit->outputs[i]->name, circuit->outputs[i]->value);\n");
    fprintf(file, "    }\n");
    fprintf(file, "    \n");
    fprintf(file, "    // Free memory\n");
    fprintf(file, "    freeCircuit(circuit);\n");
    fprintf(file, "    \n");
    fprintf(file, "    return 0;\n");
    fprintf(file, "}\n");
    
    fclose(file);
    printf("Circuit saved as C file: %s\n", filename);
}

int main() {
    // Parse the Verilog file
    Circuit* circuit = parseVerilogFile("c17 (2).v");
    
    // Count paths in the circuit
    int pathCount = countPaths(circuit);
    printf("Total number of paths in the circuit: %d\n", pathCount);
    
    // Save the circuit as a C file
    saveCircuitAsC(circuit, "circuit_c17.c");
    
    // Free memory
    freeCircuit(circuit);
    
    return 0;
}
