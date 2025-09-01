#ifndef VERILOG_PARSER_H
#define VERILOG_PARSER_H

// --- General Defines ---
#define MAX_NAME_LENGTH 64

// --- Header Declaration Defines ---
#define MAX_PORTS 200
#define MAX_SIGNALS 500 // Applies to inputs, outputs, or wires lists

// --- Gate Declaration Defines ---
#define MAX_GATE_INSTANCE_NAME_LEN MAX_NAME_LENGTH
#define MAX_GATE_BASE_NAME_LEN MAX_NAME_LENGTH
#define MAX_GATE_INPUTS 16      // Max inputs a single gate can have (adjust as needed)
#define MAX_GATE_INSTANCES 2000 // Max number of gate instances in the module (adjust for larger circuits)

// --- Data Structures ---

// Structure to hold a signal/port name (reused)
typedef struct {
    char name[MAX_NAME_LENGTH];
} Signal;

// Enumeration for gate types
typedef enum {
    GATE_UNKNOWN,
    GATE_AND,
    GATE_NAND,
    GATE_OR,
    GATE_NOR,
    GATE_XOR,
    GATE_XNOR,
    GATE_NOT,
    GATE_BUFF // Buffer
} GateType;

// Structure to hold a gate instantiation
typedef struct {
    GateType type;
    char instance_name[MAX_GATE_INSTANCE_NAME_LEN]; // e.g., "NAND2_1"
    char base_name[MAX_GATE_BASE_NAME_LEN];         // e.g., "NAND2"
    int instance_number;                            // e.g., 1
    char output_signal[MAX_NAME_LENGTH];            // The result signal
    Signal input_signals[MAX_GATE_INPUTS];          // Input signals
    int input_signal_count;
} GateInstance;


// --- Extern Declarations for Parsed Header Data ---
extern char module_name[MAX_NAME_LENGTH];
extern Signal module_ports[MAX_PORTS];
extern int module_port_count;

extern Signal input_signals[MAX_SIGNALS];
extern int input_count;

extern Signal output_signals[MAX_SIGNALS];
extern int output_count;

extern Signal wire_signals[MAX_SIGNALS];
extern int wire_count;

// --- Extern Declarations for Parsed Gate Data ---
extern GateInstance parsed_gates[MAX_GATE_INSTANCES];
extern int parsed_gate_count;


// --- Public Function Prototypes ---

/**
 * @brief Parses the specified gate-level Verilog file, including header and gate instantiations.
 *
 * Populates global data structures for module info, signals, and gate instances.
 * @param filename The path to the Verilog file.
 * @return 0 on success, 1 if file cannot be opened.
 */
int parse_verilog_module(const char *filename); // Renamed for clarity

/**
 * @brief Resets all global parsed data structures to their initial empty state.
 */
void reset_parsed_data(void);

/**
 * @brief Converts a GateType enum to its string representation.
 * @param type The GateType enum value.
 * @return A constant string representing the gate type (e.g., "NAND").
 */
const char* gate_type_to_string(GateType type);


#endif // VERILOG_PARSER_H
