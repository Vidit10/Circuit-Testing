#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Gate types
typedef enum {
    INPUT,
    OUTPUT,
    GATE,
    BRANCH
} NodeType;

typedef enum {
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    NOT,
    BUFF
} GateType;

typedef struct Node {
    int id;
    char* name;
    NodeType type;
    GateType gateType;
    bool value;
    bool visited;
    int pathCount;
    
    struct Node** inputs;
    int numInputs;
    int maxInputs;
    
    struct Node** outputs;
    int numOutputs;
    int maxOutputs;
} Node;

typedef struct {
    Node** nodes;
    int nodeCount;
    int maxNodes;
    
    Node** inputs;
    int inputCount;
    int maxInputs;
    
    Node** outputs;
    int outputCount;
    int maxOutputs;
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
Node* createNode(int id, const char* name, NodeType type) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) {
        printf("Memory allocation failed for node\n");
        exit(1);
    }
    
    node->id = id;
    node->name = strdup(name);
    node->type = type;
    node->value = false;
    node->visited = false;
    node->pathCount = 0;
    
    node->inputs = (Node**)malloc(5 * sizeof(Node*));
    node->numInputs = 0;
    node->maxInputs = 5;
    
    node->outputs = (Node**)malloc(5 * sizeof(Node*));
    node->numOutputs = 0;
    node->maxOutputs = 5;
    
    return node;
}

// Function to add an input to a node
void addInput(Node* node, Node* input) {
    // Check if input already exists
    for (int i = 0; i < node->numInputs; i++) {
        if (node->inputs[i] == input) {
            return;
        }
    }
    
    // Resize if needed
    if (node->numInputs >= node->maxInputs) {
        node->maxInputs *= 2;
        node->inputs = (Node**)realloc(node->inputs, node->maxInputs * sizeof(Node*));
    }
    
    node->inputs[node->numInputs++] = input;
    
    // Also add node as output to input
    addOutput(input, node);
}

void addOutput(Node* node, Node* output) {
    // Check if output already exists
    for (int i = 0; i < node->numOutputs; i++) {
        if (node->outputs[i] == output) {
            return;
        }
    }
    
    // Resize if needed
    if (node->numOutputs >= node->maxOutputs) {
        node->maxOutputs *= 2;
        node->outputs = (Node**)realloc(node->outputs, node->maxOutputs * sizeof(Node*));
    }
    
    node->outputs[node->numOutputs++] = output;
}

// Function to initialize a circuit
Circuit* initCircuit() {
    Circuit* circuit = (Circuit*)malloc(sizeof(Circuit));
    if (!circuit) {
        printf("Memory allocation failed for circuit\n");
        exit(1);
    }
    
    circuit->nodes = (Node**)malloc(50 * sizeof(Node*));
    circuit->nodeCount = 0;
    circuit->maxNodes = 50;
    
    circuit->inputs = (Node**)malloc(20 * sizeof(Node*));
    circuit->inputCount = 0;
    circuit->maxInputs = 20;
    
    circuit->outputs = (Node**)malloc(20 * sizeof(Node*));
    circuit->outputCount = 0;
    circuit->maxOutputs = 20;
    
    return circuit;
}

// Function to add a node to the circuit
void addNode(Circuit* circuit, Node* node) {
    // Resize if needed
    if (circuit->nodeCount >= circuit->maxNodes) {
        circuit->maxNodes *= 2;
        circuit->nodes = (Node**)realloc(circuit->nodes, circuit->maxNodes * sizeof(Node*));
    }
    
    circuit->nodes[circuit->nodeCount++] = node;
    
    // Add to input or output list if applicable
    if (node->type == INPUT) {
        if (circuit->inputCount >= circuit->maxInputs) {
            circuit->maxInputs *= 2;
            circuit->inputs = (Node**)realloc(circuit->inputs, circuit->maxInputs * sizeof(Node*));
        }
        circuit->inputs[circuit->inputCount++] = node;
    } else if (node->type == OUTPUT) {
        if (circuit->outputCount >= circuit->maxOutputs) {
            circuit->maxOutputs *= 2;
            circuit->outputs = (Node**)realloc(circuit->outputs, circuit->maxOutputs * sizeof(Node*));
        }
        circuit->outputs[circuit->outputCount++] = node;
    }
}

Node* findNode(Circuit* circuit, const char* name) {
    for (int i = 0; i < circuit->nodeCount; i++) {
        if (strcmp(circuit->nodes[i]->name, name) == 0) {
            return circuit->nodes[i];
        }
    }
    return NULL;
}

Circuit* parseVerilogFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        exit(1);
    }
    
    Circuit* circuit = initCircuit();
    char line[256];
    char moduleName[50];
    bool foundModule = false;
    
    // Buffer for collecting multi-line declarations
    char declarationBuffer[2048] = {0};
    bool collectingInputs = false;
    bool collectingOutputs = false;
    bool collectingWires = false;
    
    // Parse module declaration and find input/output declarations
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        if (line[0] == '/' || line[0] == '\n' || strlen(line) <= 1) {
            continue;
        }
        
        // Parse module name
        if (!foundModule && strstr(line, "module")) {
            sscanf(line, "module %[^( ]", moduleName);
            foundModule = true;
            continue;
        }
        
        // Handle input declarations (potentially multi-line)
        if (strstr(line, "input") || collectingInputs) {
            if (!collectingInputs) {
                collectingInputs = true;
                strcpy(declarationBuffer, "");
            }
            
            // Append this line to our buffer
            strcat(declarationBuffer, line);
            
            // Check if declaration is complete (ends with semicolon)
            if (strchr(line, ';')) {
                collectingInputs = false;
                
                // Extract the input list
                char* inputStart = strstr(declarationBuffer, "input") + 5;
                char* semicolon = strchr(inputStart, ';');
                *semicolon = '\0'; // Terminate at semicolon
                
                // Now parse the complete input list
                char* saveptr;
                char* token = strtok_r(inputStart, ",", &saveptr);
                
                while (token != NULL) {
                    // Trim whitespace
                    char* trimmed = token;
                    while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\n') trimmed++;
                    
                    char* end = trimmed + strlen(trimmed) - 1;
                    while (end > trimmed && (*end == ' ' || *end == '\t' || *end == '\n')) {
                        *end = '\0';
                        end--;
                    }
                    
                    if (strlen(trimmed) > 0) {
                        Node* node = createNode(circuit->nodeCount, trimmed, INPUT);
                        addNode(circuit, node);
                    }
                    
                    token = strtok_r(NULL, ",", &saveptr);
                }
            }
            continue;
        }
        
        // Handle output declarations (potentially multi-line)
        if (strstr(line, "output") || collectingOutputs) {
            if (!collectingOutputs) {
                collectingOutputs = true;
                strcpy(declarationBuffer, "");
            }
            
            // Append this line to our buffer
            strcat(declarationBuffer, line);
            
            // Check if declaration is complete (ends with semicolon)
            if (strchr(line, ';')) {
                collectingOutputs = false;
                
                // Extract the output list
                char* outputStart = strstr(declarationBuffer, "output") + 6;
                char* semicolon = strchr(outputStart, ';');
                *semicolon = '\0'; // Terminate at semicolon
                
                // Now parse the complete output list
                char* saveptr;
                char* token = strtok_r(outputStart, ",", &saveptr);
                
                while (token != NULL) {
                    // Trim whitespace
                    char* trimmed = token;
                    while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\n') trimmed++;
                    
                    char* end = trimmed + strlen(trimmed) - 1;
                    while (end > trimmed && (*end == ' ' || *end == '\t' || *end == '\n')) {
                        *end = '\0';
                        end--;
                    }
                    
                    if (strlen(trimmed) > 0) {
                        Node* node = createNode(circuit->nodeCount, trimmed, OUTPUT);
                        addNode(circuit, node);
                    }
                    
                    token = strtok_r(NULL, ",", &saveptr);
                }
            }
            continue;
        }
        
        // Handle wire declarations (potentially multi-line)
        if (strstr(line, "wire") || collectingWires) {
            if (!collectingWires) {
                collectingWires = true;
                strcpy(declarationBuffer, "");
            }
            
            // Append this line to our buffer
            strcat(declarationBuffer, line);
            
            // Check if declaration is complete (ends with semicolon)
            if (strchr(line, ';')) {
                collectingWires = false;
                
                // Extract the wire list
                char* wireStart = strstr(declarationBuffer, "wire") + 4;
                char* semicolon = strchr(wireStart, ';');
                *semicolon = '\0'; // Terminate at semicolon
                
                // Now parse the complete wire list
                char* saveptr;
                char* token = strtok_r(wireStart, ",", &saveptr);
                
                while (token != NULL) {
                    // Trim whitespace
                    char* trimmed = token;
                    while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\n') trimmed++;
                    
                    char* end = trimmed + strlen(trimmed) - 1;
                    while (end > trimmed && (*end == ' ' || *end == '\t' || *end == '\n')) {
                        *end = '\0';
                        end--;
                    }
                    
                    if (strlen(trimmed) > 0) {
                        Node* node = createNode(circuit->nodeCount, trimmed, BRANCH);
                        addNode(circuit, node);
                    }
                    
                    token = strtok_r(NULL, ",", &saveptr);
                }
            }
            continue;
        }
        
        // If we've found all declarations, start parsing gates
        if (strstr(line, "nand") || strstr(line, "and") || 
            strstr(line, "or") || strstr(line, "nor") || 
            strstr(line, "xor") || strstr(line, "not") ||
            strstr(line, "buff")) {
            break;
        }
    }
    
    // Parse gate declarations
    do {
        // Skip comments, empty lines, and endmodule
        if (line[0] == '/' || line[0] == '\n' || strlen(line) <= 1 || strstr(line, "endmodule")) {
            continue;
        }
        
        // Check if this is a gate declaration
        if (strstr(line, "(") && strstr(line, ")")) {
            char gatetype[20] = {0};
            char gatename[50] = {0};
            char connections[200] = {0};
            
            // Extract gate type, name, and connections
            if (sscanf(line, "%19s %49s (%199[^)])", gatetype, gatename, connections) == 3) {
                // Determine gate type
                GateType gateType;
                if (strcmp(gatetype, "nand") == 0) gateType = NAND;
                else if (strcmp(gatetype, "and") == 0) gateType = AND;
                else if (strcmp(gatetype, "or") == 0) gateType = OR;
                else if (strcmp(gatetype, "nor") == 0) gateType = NOR;
                else if (strcmp(gatetype, "xor") == 0) gateType = XOR;
                else if (strcmp(gatetype, "xnor") == 0) gateType = XNOR;
                else if (strcmp(gatetype, "not") == 0) gateType = NOT;
                else if (strcmp(gatetype, "buff") == 0) gateType = BUFF;
                else {
                    printf("Unknown gate type: %s\n", gatetype);
                    continue;
                }
                
                // Create gate node
                Node* gateNode = createNode(circuit->nodeCount, gatename, GATE);
                gateNode->gateType = gateType;
                addNode(circuit, gateNode);
                
                // Parse connections
                char* connList = strdup(connections);
                char* saveptr;
                char* token = strtok_r(connList, ",", &saveptr);
                int connIndex = 0;
                
                while (token != NULL) {
                    // Trim token
                    char* trimmed = token;
                    while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
                    
                    char* trimEnd = trimmed + strlen(trimmed) - 1;
                    while (trimEnd > trimmed && (*trimEnd == ' ' || *trimEnd == '\t')) {
                        *trimEnd = '\0';
                        trimEnd--;
                    }
                    
                    if (strlen(trimmed) > 0) {
                        Node* connNode = findNode(circuit, trimmed);
                        
                        if (!connNode) {
                            printf("Warning: Node %s not found, creating as BRANCH\n", trimmed);
                            connNode = createNode(circuit->nodeCount, trimmed, BRANCH);
                            addNode(circuit, connNode);
                        }
                        
                        if (connIndex == 0) {
                            // First connection is the output
                            addInput(connNode, gateNode);
                        } else {
                            // Subsequent connections are inputs
                            addInput(gateNode, connNode);
                        }
                    }
                    
                    token = strtok_r(NULL, ",", &saveptr);
                    connIndex++;
                }
                
                free(connList);
            }
        }
    } while (fgets(line, sizeof(line), file));
    
    fclose(file);
    return circuit;
}

bool evaluateNode(Node* node) {
    if (node->type == INPUT) {
        return node->value;
    }
    
    if (node->numInputs == 0) {
        return false;
    }
    
    bool result = true;
    
    switch (node->gateType) {
        case AND:
            result = true;
            for (int i = 0; i < node->numInputs; i++) {
                result = result && node->inputs[i]->value;
            }
            break;
            
        case NAND:
            result = true;
            for (int i = 0; i < node->numInputs; i++) {
                result = result && node->inputs[i]->value;
            }
            result = !result;
            break;
            
        case OR:
            result = false;
            for (int i = 0; i < node->numInputs; i++) {
                result = result || node->inputs[i]->value;
            }
            break;
            
        case NOR:
            result = false;
            for (int i = 0; i < node->numInputs; i++) {
                result = result || node->inputs[i]->value;
            }
            result = !result;
            break;
            
        case XOR:
            result = false;
            for (int i = 0; i < node->numInputs; i++) {
                result = result ^ node->inputs[i]->value;
            }
            break;
            
        case XNOR:
            result = false;
            for (int i = 0; i < node->numInputs; i++) {
                result = result ^ node->inputs[i]->value;
            }
            result = !result;
            break;
            
        case NOT:
            if (node->numInputs > 0) {
                result = !node->inputs[0]->value;
            }
            break;
            
        case BUFF:
            if (node->numInputs > 0) {
                result = node->inputs[0]->value;
            }
            break;
            
        default:
            result = false;
    }
    
    return result;
}

void simulateCircuit(Circuit* circuit, bool* inputValues) {
    // Set input values
    for (int i = 0; i < circuit->inputCount; i++) {
        circuit->inputs[i]->value = inputValues[i];
        circuit->inputs[i]->visited = true;
    }
    
    // Reset visited flags for non-input nodes
    for (int i = 0; i < circuit->nodeCount; i++) {
        if (circuit->nodes[i]->type != INPUT) {
            circuit->nodes[i]->visited = false;
        }
    }
    
    // Create a queue for topological evaluation
    Node** queue = (Node**)malloc(circuit->nodeCount * sizeof(Node*));
    int queueSize = 0;
    
    // Add all nodes with all inputs visited to the queue
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        
        if (node->type == INPUT) {
            // Add all nodes that this input connects to
            for (int j = 0; j < node->numOutputs; j++) {
                // Check if all inputs of this node are visited
                bool allInputsVisited = true;
                for (int k = 0; k < node->outputs[j]->numInputs; k++) {
                    if (!node->outputs[j]->inputs[k]->visited) {
                        allInputsVisited = false;
                        break;
                    }
                }
                
                if (allInputsVisited && !node->outputs[j]->visited) {
                    queue[queueSize++] = node->outputs[j];
                }
            }
        }
    }
    
    // Process the queue
    while (queueSize > 0) {
        // Pop a node from the queue
        Node* node = queue[0];
        queueSize--;
        
        // Shift the queue
        for (int i = 0; i < queueSize; i++) {
            queue[i] = queue[i + 1];
        }
        
        // Evaluate the node
        node->value = evaluateNode(node);
        node->visited = true;
        
        // Add all nodes that this node connects to
        for (int i = 0; i < node->numOutputs; i++) {
            Node* outputNode = node->outputs[i];
            
            if (!outputNode->visited) {
                // Check if all inputs of this node are visited
                bool allInputsVisited = true;
                for (int j = 0; j < outputNode->numInputs; j++) {
                    if (!outputNode->inputs[j]->visited) {
                        allInputsVisited = false;
                        break;
                    }
                }
                
                if (allInputsVisited) {
                    // Check if already in queue
                    bool inQueue = false;
                    for (int j = 0; j < queueSize; j++) {
                        if (queue[j] == outputNode) {
                            inQueue = true;
                            break;
                        }
                    }
                    
                    if (!inQueue) {
                        queue[queueSize++] = outputNode;
                    }
                }
            }
        }
    }
    
    free(queue);
}

void topologicalSort(Circuit* circuit, Node** sorted, int* sortedCount) {
    // Reset visited flags
    for (int i = 0; i < circuit->nodeCount; i++) {
        circuit->nodes[i]->visited = false;
    }
    
    *sortedCount = 0;
    
    // Helper function for DFS
    void dfs(Node* node) {
        node->visited = true;
        
        // Visit all outputs first
        for (int i = 0; i < node->numOutputs; i++) {
            if (!node->outputs[i]->visited) {
                dfs(node->outputs[i]);
            }
        }
        
        // Add to sorted list
        sorted[(*sortedCount)++] = node;
    }
    
    // Start DFS from input nodes
    for (int i = 0; i < circuit->inputCount; i++) {
        if (!circuit->inputs[i]->visited) {
            dfs(circuit->inputs[i]);
        }
    }
    
    // Reverse the sorted list to get correct topological order
    for (int i = 0; i < *sortedCount / 2; i++) {
        Node* temp = sorted[i];
        sorted[i] = sorted[*sortedCount - 1 - i];
        sorted[*sortedCount - 1 - i] = temp;
    }
}

int countPaths(Circuit* circuit) {
    // Initialize path counts
    for (int i = 0; i < circuit->nodeCount; i++) {
        circuit->nodes[i]->pathCount = 0;
    }
    
    // Set path count for input nodes to 1
    for (int i = 0; i < circuit->inputCount; i++) {
        circuit->inputs[i]->pathCount = 1;
    }
    
    // Get topological order
    Node** sorted = (Node**)malloc(circuit->nodeCount * sizeof(Node*));
    int sortedCount = 0;
    topologicalSort(circuit, sorted, &sortedCount);
    
    // Calculate path counts in topological order
    for (int i = 0; i < sortedCount; i++) {
        Node* node = sorted[i];
        
        if (node->type != INPUT) {
            // Sum path counts from all inputs
            for (int j = 0; j < node->numInputs; j++) {
                node->pathCount += node->inputs[j]->pathCount;
            }
        }
    }
    
    // Sum path counts for output nodes
    int totalPaths = 0;
    for (int i = 0; i < circuit->outputCount; i++) {
        totalPaths += circuit->outputs[i]->pathCount;
    }
    
    free(sorted);
    return totalPaths;
}

void printCircuit(Circuit* circuit) {
    printf("Circuit Details:\n");
    printf("Total nodes: %d\n", circuit->nodeCount);
    printf("Input nodes: %d\n", circuit->inputCount);
    printf("Output nodes: %d\n", circuit->outputCount);
    
    printf("\nInput Nodes:\n");
    for (int i = 0; i < circuit->inputCount; i++) {
        printf("  %s (ID: %d)\n", circuit->inputs[i]->name, circuit->inputs[i]->id);
    }
    
    printf("\nOutput Nodes:\n");
    for (int i = 0; i < circuit->outputCount; i++) {
        printf("  %s (ID: %d)\n", circuit->outputs[i]->name, circuit->outputs[i]->id);
    }
    
    printf("\nAll Nodes and Connections:\n");
    for (int i = 0; i < circuit->nodeCount; i++) {
        Node* node = circuit->nodes[i];
        
        printf("Node: %s (ID: %d, Type: ", node->name, node->id);
        
        switch (node->type) {
            case INPUT: printf("INPUT"); break;
            case OUTPUT: printf("OUTPUT"); break;
            case GATE: 
                printf("GATE (");
                switch (node->gateType) {
                    case AND: printf("AND"); break;
                    case NAND: printf("NAND"); break;
                    case OR: printf("OR"); break;
                    case NOR: printf("NOR"); break;
                    case XOR: printf("XOR"); break;
                    case XNOR: printf("XNOR"); break;
                    case NOT: printf("NOT"); break;
                    case BUFF: printf("BUFF"); break;
                }
                printf(")");
                break;
            case BRANCH: printf("BRANCH"); break;
        }
        printf(")\n");
        
        printf("  Inputs (%d):", node->numInputs);
        for (int j = 0; j < node->numInputs; j++) {
            printf(" %s", node->inputs[j]->name);
        }
        printf("\n");
        
        printf("  Outputs (%d):", node->numOutputs);
        for (int j = 0; j < node->numOutputs; j++) {
            printf(" %s", node->outputs[j]->name);
        }
        printf("\n\n");
    }
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

int main() {
    // Parse the Verilog file
    Circuit* circuit = parseVerilogFile("c17.v");
    
    // Print circuit details
    printCircuit(circuit);
    
    // Count paths in the circuit
    int pathCount = countPaths(circuit);
    printf("Total number of paths in the circuit: %d\n", pathCount);
    
    // Simulate the circuit with all inputs set to 1
    bool inputValues[5] = {1, 1, 1, 1, 1};
    simulateCircuit(circuit, inputValues);
    
    // Print output values
    printf("\nOutput values after simulation (all inputs = 1):\n");
    for (int i = 0; i < circuit->outputCount; i++) {
        printf("%s: %d\n", circuit->outputs[i]->name, circuit->outputs[i]->value);
    }
    
    // Free memory
    freeCircuit(circuit);
    
    return 0;
}
