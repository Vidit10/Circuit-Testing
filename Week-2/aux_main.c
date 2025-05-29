#include <stdio.h>
#include "verilog_parser.h" // Includes all necessary declarations

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <verilog_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    printf("Attempting to parse Verilog file: %s\n", filename);

    if (parse_verilog_module(filename) != 0) { // Use the renamed function
        fprintf(stderr, "An error occurred during parsing.\n");
        return 1;
    }

    printf("\n## Parsed Verilog Information (from main.c)\n\n");

    // --- Print Header Information (same as before) ---
    printf("### Module Name:\n%s\n\n", module_name);

    printf("### Module Ports (from module declaration):\n");
    if (module_port_count == 0) printf("None\n");
    for (int i = 0; i < module_port_count; i++) printf("- %s\n", module_ports[i].name);
    printf("\n");

    printf("### Input Signals:\n");
    if (input_count == 0) printf("None\n");
    for (int i = 0; i < input_count; i++) printf("- %s\n", input_signals[i].name);
    printf("\n");

    printf("### Output Signals:\n");
    if (output_count == 0) printf("None\n");
    for (int i = 0; i < output_count; i++) {
        printf("- %s\n", output_signals[i].name);
        // if (i < output_count - 1) printf(";\n"); else printf("\n");
    }
    printf("\n");

    printf("### Wire Signals:\n");
    if (wire_count == 0) printf("None\n");
    for (int i = 0; i < wire_count; i++) printf("- %s\n", wire_signals[i].name);
    printf("\n");

    // --- NEW: Print Parsed Gate Information ---
    printf("### Gate Instantiations:\n");
    if (parsed_gate_count == 0) {
        printf("None\n");
    } else {
        for (int i = 0; i < parsed_gate_count; i++) {
            GateInstance *gate = &parsed_gates[i];
            printf("Gate %d:\n", i + 1);
            printf("  - Type: %s\n", gate_type_to_string(gate->type)); // Use new helper
            printf("  - Instance Name: %s\n", gate->instance_name);
            printf("  - Base Name: %s\n", gate->base_name);
            printf("  - Instance Number: %d\n", gate->instance_number);
            printf("  - Output Signal: %s\n", gate->output_signal);
            printf("  - Input Signals (%d):\n", gate->input_signal_count);
            if (gate->input_signal_count == 0) {
                printf("    None\n");
            } else {
                for (int j = 0; j < gate->input_signal_count; j++) {
                    printf("    - %s\n", gate->input_signals[j].name);
                }
            }
            printf("\n");
        }
    }

    return 0;
}
