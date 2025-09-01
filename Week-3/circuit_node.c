#include "circuit_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Circuit* create_circuit(void) {
    Circuit* circuit = (Circuit*)malloc(sizeof(Circuit));
    if (!circuit) return NULL;
    
    circuit->node_count = 0;
    circuit->pi_count = 0;
    circuit->po_count = 0;
    circuit->simulation_stable = false;
    circuit->iteration_count = 0;
    
    // Initialize all nodes
    for (int i = 0; i < MAX_NODES; i++) {
        circuit->nodes[i].name[0] = '\0';
        circuit->nodes[i].id = -1;
        circuit->nodes[i].type = NODE_GATE;
        circuit->nodes[i].gate_type = GATE_UNKNOWN;
        circuit->nodes[i].value = LOGIC_X;
        circuit->nodes[i].fanin_list = NULL;
        circuit->nodes[i].fanout_list = NULL;
        circuit->nodes[i].fanin_count = 0;
        circuit->nodes[i].fanout_count = 0;
        circuit->nodes[i].gate_instance[0] = '\0';
        circuit->nodes[i].is_evaluated = false;
    }
    
    return circuit;
}

void destroy_circuit(Circuit* circuit) {
    if (!circuit) return;
    
    // Free all connection lists
    for (int i = 0; i < circuit->node_count; i++) {
        ConnectionNode* current = circuit->nodes[i].fanin_list;
        while (current) {
            ConnectionNode* next = current->next;
            free(current);
            current = next;
        }
        
        current = circuit->nodes[i].fanout_list;
        while (current) {
            ConnectionNode* next = current->next;
            free(current);
            current = next;
        }
    }
    
    free(circuit);
}

int add_node(Circuit* circuit, const char* name, NodeType type) {
    if (!circuit || !name || circuit->node_count >= MAX_NODES) {
        return -1;
    }
    
    // Check if node already exists
    int existing_id = find_node_by_name(circuit, name);
    if (existing_id != -1) {
        // Update type if more specific (PI/PO take precedence over GATE)
        if (type == NODE_PI || type == NODE_PO) {
            circuit->nodes[existing_id].type = type;
            
            // Add to appropriate lists if not already there
            if (type == NODE_PI) {
                bool found = false;
                for (int i = 0; i < circuit->pi_count; i++) {
                    if (circuit->primary_inputs[i] == existing_id) {
                        found = true;
                        break;
                    }
                }
                if (!found && circuit->pi_count < MAX_SIGNALS) {
                    circuit->primary_inputs[circuit->pi_count++] = existing_id;
                }
            } else if (type == NODE_PO) {
                bool found = false;
                for (int i = 0; i < circuit->po_count; i++) {
                    if (circuit->primary_outputs[i] == existing_id) {
                        found = true;
                        break;
                    }
                }
                if (!found && circuit->po_count < MAX_SIGNALS) {
                    circuit->primary_outputs[circuit->po_count++] = existing_id;
                }
            }
        }
        return existing_id;
    }
    
    // Create new node
    int node_id = circuit->node_count;
    CircuitNode* node = &circuit->nodes[node_id];
    
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->name[sizeof(node->name) - 1] = '\0';
    node->id = node_id;
    node->type = type;
    node->value = LOGIC_X;
    node->fanin_list = NULL;
    node->fanout_list = NULL;
    node->fanin_count = 0;
    node->fanout_count = 0;
    node->is_evaluated = false;
    
    // Add to primary input/output lists if applicable
    if (type == NODE_PI && circuit->pi_count < MAX_SIGNALS) {
        circuit->primary_inputs[circuit->pi_count++] = node_id;
    } else if (type == NODE_PO && circuit->po_count < MAX_SIGNALS) {
        circuit->primary_outputs[circuit->po_count++] = node_id;
    }
    
    circuit->node_count++;
    return node_id;
}

int find_node_by_name(Circuit* circuit, const char* name) {
    if (!circuit || !name) return -1;
    
    for (int i = 0; i < circuit->node_count; i++) {
        if (strcmp(circuit->nodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_node_by_id(Circuit* circuit, int id) {
    if (!circuit || id < 0 || id >= circuit->node_count) {
        return -1;
    }
    return id;
}

bool add_connection(Circuit* circuit, int from_node_id, int to_node_id) {
    if (!circuit || from_node_id < 0 || to_node_id < 0 || 
        from_node_id >= circuit->node_count || to_node_id >= circuit->node_count) {
        return false;
    }
    
    // Add to fanout list of source node
    ConnectionNode* fanout_conn = (ConnectionNode*)malloc(sizeof(ConnectionNode));
    if (!fanout_conn) return false;
    
    fanout_conn->node_id = to_node_id;
    fanout_conn->next = circuit->nodes[from_node_id].fanout_list;
    circuit->nodes[from_node_id].fanout_list = fanout_conn;
    circuit->nodes[from_node_id].fanout_count++;
    
    // Add to fanin list of destination node
    ConnectionNode* fanin_conn = (ConnectionNode*)malloc(sizeof(ConnectionNode));
    if (!fanin_conn) return false;
    
    fanin_conn->node_id = from_node_id;
    fanin_conn->next = circuit->nodes[to_node_id].fanin_list;
    circuit->nodes[to_node_id].fanin_list = fanin_conn;
    circuit->nodes[to_node_id].fanin_count++;
    
    return true;
}

void add_branch_nodes(Circuit* circuit) {
    if (!circuit) return;
    
    // Find nodes with fanout > 1 and create branch nodes
    for (int i = 0; i < circuit->node_count; i++) {
        CircuitNode* node = &circuit->nodes[i];
        
        if (node->fanout_count > 1 && node->type != NODE_BRNH) {
            // This node needs branch nodes
            ConnectionNode* fanout = node->fanout_list;
            ConnectionNode* first_fanout = fanout;
            
            // Keep first connection as is, create branch nodes for others
            fanout = fanout->next;
            int branch_counter = 1;
            
            while (fanout) {
                // Create branch node name
                char branch_name[64];
                // snprintf(branch_name, sizeof(branch_name), "%s_b%d", node->name, branch_counter++);
                int written = snprintf(branch_name, sizeof(branch_name), "%s_b%d", node->name, branch_counter++);
                if (written >= sizeof(branch_name)) {
                    // Name was truncated, but continue anyway
                    branch_name[sizeof(branch_name) - 1] = '\0';
                }
                
                // Add branch node
                int branch_id = add_node(circuit, branch_name, NODE_BRNH);
                if (branch_id == -1) break;
                
                // Connect original node to branch node
                add_connection(circuit, i, branch_id);
                
                // Redirect original connection through branch node
                int target_node = fanout->node_id;
                
                // Remove original connection from target's fanin
                ConnectionNode** fanin_ptr = &circuit->nodes[target_node].fanin_list;
                while (*fanin_ptr) {
                    if ((*fanin_ptr)->node_id == i) {
                        ConnectionNode* to_remove = *fanin_ptr;
                        *fanin_ptr = (*fanin_ptr)->next;
                        free(to_remove);
                        circuit->nodes[target_node].fanin_count--;
                        break;
                    }
                    fanin_ptr = &(*fanin_ptr)->next;
                }
                
                // Add connection from branch node to target
                add_connection(circuit, branch_id, target_node);
                
                fanout = fanout->next;
            }
            
            // Clean up original fanout list (keep only first connection)
            fanout = first_fanout->next;
            while (fanout) {
                ConnectionNode* next = fanout->next;
                free(fanout);
                fanout = next;
            }
            first_fanout->next = NULL;
            node->fanout_count = 1;
        }
    }
}

bool simulate_circuit(Circuit* circuit) {
    if (!circuit) return false;
    
    const int MAX_ITERATIONS = 1000;
    bool changes_occurred = true;
    circuit->iteration_count = 0;
    circuit->simulation_stable = false;
    
    // Reset evaluation flags
    for (int i = 0; i < circuit->node_count; i++) {
        circuit->nodes[i].is_evaluated = false;
    }
    
    while (changes_occurred && circuit->iteration_count < MAX_ITERATIONS) {
        changes_occurred = false;
        circuit->iteration_count++;
        
        // Evaluate all nodes that have gate logic or are branch nodes
        for (int i = 0; i < circuit->node_count; i++) {
            CircuitNode* node = &circuit->nodes[i];
            
            // Skip primary inputs - they keep their set values
            if (node->type == NODE_PI) {
                continue;
            }
            
            if (node->type == NODE_BRNH) {
                // Branch nodes simply pass through the value
                if (node->fanin_list) {
                    SignalValue input_value = circuit->nodes[node->fanin_list->node_id].value;
                    if (node->value != input_value) {
                        node->value = input_value;
                        changes_occurred = true;
                    }
                }
            } else if (node->gate_type != GATE_UNKNOWN) {
                // This is a gate node (could be GATE, PO, or any other type with gate logic)
                // Collect input values
                SignalValue inputs[MAX_GATE_INPUTS];
                int input_count = 0;
                
                ConnectionNode* fanin = node->fanin_list;
                while (fanin && input_count < MAX_GATE_INPUTS) {
                    inputs[input_count++] = circuit->nodes[fanin->node_id].value;
                    fanin = fanin->next;
                }
                
                // Evaluate gate
                SignalValue new_value = LOGIC_X;
                switch (node->gate_type) {
                    case GATE_AND:  new_value = evaluate_and(inputs, input_count); break;
                    case GATE_NAND: new_value = evaluate_nand(inputs, input_count); break;
                    case GATE_OR:   new_value = evaluate_or(inputs, input_count); break;
                    case GATE_NOR:  new_value = evaluate_nor(inputs, input_count); break;
                    case GATE_XOR:
                        if (input_count == 2) new_value = evaluate_xor2(inputs[0], inputs[1]);
                        break;
                    case GATE_XNOR:
                        if (input_count == 2) new_value = evaluate_xnor2(inputs[0], inputs[1]);
                        break;
                    case GATE_NOT:
                        if (input_count == 1) new_value = evaluate_not1(inputs[0]);
                        break;
                    case GATE_BUFF:
                        if (input_count == 1) new_value = evaluate_buff1(inputs[0]);
                        break;
                    default:
                        new_value = LOGIC_X;
                        break;
                }
                
                // Update value if changed
                if (node->value != new_value) {
                    node->value = new_value;
                    changes_occurred = true;
                }
                
                node->is_evaluated = true;
            }
        }
    }
    
    circuit->simulation_stable = !changes_occurred;
    return circuit->simulation_stable;
}

void set_primary_inputs(Circuit* circuit, const SignalValue* input_values) {
    if (!circuit || !input_values) return;
    
    for (int i = 0; i < circuit->pi_count; i++) {
        int node_id = circuit->primary_inputs[i];
        circuit->nodes[node_id].value = input_values[i];
    }
}

void reset_simulation(Circuit* circuit) {
    if (!circuit) return;
    
    for (int i = 0; i < circuit->node_count; i++) {
        if (circuit->nodes[i].type != NODE_PI) {
            circuit->nodes[i].value = LOGIC_X;
        }
        circuit->nodes[i].is_evaluated = false;
    }
    
    circuit->simulation_stable = false;
    circuit->iteration_count = 0;
}

void print_circuit_info(Circuit* circuit) {
    if (!circuit) return;
    
    printf("=== Circuit Information ===\n");
    printf("Total Nodes: %d\n", circuit->node_count);
    printf("Primary Inputs: %d\n", circuit->pi_count);
    printf("Primary Outputs: %d\n", circuit->po_count);
    
    // Count gate nodes
    int gate_count = 0;
    int branch_count = 0;
    for (int i = 0; i < circuit->node_count; i++) {
        if (circuit->nodes[i].type == NODE_GATE) gate_count++;
        else if (circuit->nodes[i].type == NODE_BRNH) branch_count++;
    }
    printf("Gate Nodes: %d\n", gate_count);
    printf("Branch Nodes: %d\n", branch_count);
    printf("\n");
}

void print_node_values(Circuit* circuit) {
    if (!circuit) return;
    
    printf("=== Node Values ===\n");
    printf("%-15s | %-4s | %-8s | %-6s | %-8s\n", "Name", "ID", "Type", "Value", "Gate");
    printf("----------------+------+----------+--------+----------\n");
    
    for (int i = 0; i < circuit->node_count; i++) {
        CircuitNode* node = &circuit->nodes[i];
        const char* type_str = (node->type == NODE_PI) ? "PI" :
                              (node->type == NODE_PO) ? "PO" :
                              (node->type == NODE_BRNH) ? "BRNH" : "GATE";
        const char* gate_str = (node->gate_type != GATE_UNKNOWN) ? gate_type_to_string(node->gate_type) : "-";
        
        printf("%-15s | %-4d | %-8s | %-6c | %-8s\n", 
               node->name, node->id, type_str, 
               signal_value_to_char(node->value), gate_str);
    }
    printf("\n");
}

void print_connections(Circuit* circuit) {
    if (!circuit) return;
    
    printf("=== Circuit Connections ===\n");
    for (int i = 0; i < circuit->node_count; i++) {
        CircuitNode* node = &circuit->nodes[i];
        printf("Node %s (ID:%d):\n", node->name, node->id);
        
        printf("  Fanin (%d): ", node->fanin_count);
        ConnectionNode* conn = node->fanin_list;
        while (conn) {
            printf("%s(%d) ", circuit->nodes[conn->node_id].name, conn->node_id);
            conn = conn->next;
        }
        printf("\n");
        
        printf("  Fanout (%d): ", node->fanout_count);
        conn = node->fanout_list;
        while (conn) {
            printf("%s(%d) ", circuit->nodes[conn->node_id].name, conn->node_id);
            conn = conn->next;
        }
        printf("\n\n");
    }
}