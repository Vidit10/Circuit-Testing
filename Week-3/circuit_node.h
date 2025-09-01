#ifndef CIRCUIT_NODE_H
#define CIRCUIT_NODE_H

#include "gate_logic.h"
#include "verilog_parser.h"
#include <stdbool.h>

#define MAX_NODES 1000
#define MAX_CONNECTIONS 50

// Node types according to ISCAS format
typedef enum {
    NODE_PI,      // Primary Input
    NODE_PO,      // Primary Output  
    NODE_BRNH,    // Branch node (fanout point)
    NODE_GATE     // Gate output node
} NodeType;

// Forward declaration
typedef struct ConnectionNode ConnectionNode;

// Connection node structure for linked lists
struct ConnectionNode {
    int node_id;              // ID of connected node
    ConnectionNode* next;     // Next connection in list
};

// Main circuit node structure
typedef struct {
    char name[64];            // Node name (e.g., "N1", "N10", "N22")
    int id;                   // Unique integer ID
    NodeType type;            // Node type
    GateType gate_type;       // Gate type if this is a gate node
    SignalValue value;        // Current logic value
    
    // Fanin and fanout lists
    ConnectionNode* fanin_list;   // List of nodes driving this node
    ConnectionNode* fanout_list;  // List of nodes driven by this node
    int fanin_count;
    int fanout_count;
    
    // For gate nodes
    char gate_instance[64];   // Gate instance name
    bool is_evaluated;        // Simulation flag
} CircuitNode;

// Main circuit structure
typedef struct {
    CircuitNode nodes[MAX_NODES];  // 1D array of nodes
    int node_count;
    
    // Quick access arrays - store node IDs
    int primary_inputs[MAX_SIGNALS];   // Indices into nodes array
    int primary_outputs[MAX_SIGNALS];  // Indices into nodes array
    int pi_count;
    int po_count;
    
    // Simulation state
    bool simulation_stable;
    int iteration_count;
} Circuit;

// Function declarations
Circuit* create_circuit(void);
void destroy_circuit(Circuit* circuit);

int add_node(Circuit* circuit, const char* name, NodeType type);
int find_node_by_name(Circuit* circuit, const char* name);
int find_node_by_id(Circuit* circuit, int id);

bool add_connection(Circuit* circuit, int from_node_id, int to_node_id);
void add_branch_nodes(Circuit* circuit);

bool simulate_circuit(Circuit* circuit);
void set_primary_inputs(Circuit* circuit, const SignalValue* input_values);
void reset_simulation(Circuit* circuit);

void print_circuit_info(Circuit* circuit);
void print_node_values(Circuit* circuit);
void print_connections(Circuit* circuit);

#endif // CIRCUIT_NODE_H