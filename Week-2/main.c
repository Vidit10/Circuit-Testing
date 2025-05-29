#include <stdio.h>
#include "verilog_parser.h" // Include your parser's header

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <verilog_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    printf("Attempting to parse Verilog file: %s\n", filename);

    // Call the parsing function from your module
    if (parse_verilog_header_declarations(filename) != 0) {
        fprintf(stderr, "An error occurred during parsing.\n");
        return 1;
    }

    // Access the parsed data (which are global variables from verilog_parser.h)
    printf("\n## Parsed Verilog Information (from main_app.c)\n\n");

    printf("### Module Name:\n%s\n\n", module_name);

    printf("### Module Ports (from module declaration):\n");
    if (module_port_count == 0) {
        printf("None\n");
    } else {
        for (int i = 0; i < module_port_count; i++) {
            printf("- %s\n", module_ports[i].name);
        }
    }
    printf("\n");

    printf("### Input Signals:\n");
    if (input_count == 0) {
        printf("None\n");
    } else {
        for (int i = 0; i < input_count; i++) {
            printf("- %s\n", input_signals[i].name);
        }
    }
    printf("\n");

    printf("### Output Signals:\n");
    if (output_count == 0) {
        printf("None\n");
    } else {
        for (int i = 0; i < output_count; i++) {
            printf("- %s\n", output_signals[i].name);
        }
    }
    printf("\n");

    printf("### Wire Signals:\n");
    if (wire_count == 0) {
        printf("None\n");
    } else {
        for (int i = 0; i < wire_count; i++) {
            printf("- %s\n", wire_signals[i].name);
        }
    }
    printf("\n");

    return 0;
}
