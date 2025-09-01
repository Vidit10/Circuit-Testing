#include "gate_logic.h"
#include <stdio.h> // For potential error messages, though not strictly used in logic here

char signal_value_to_char(SignalValue val) {
    switch (val) {
        case LOGIC_0: return '0';
        case LOGIC_1: return '1';
        case LOGIC_X: return 'X';
        default:      return '?'; // Should not happen
    }
}

// --- AND gate logic ---
// Output is LOGIC_0 if any input is LOGIC_0.
// Output is LOGIC_X if any input is LOGIC_X and no input is LOGIC_0.
// Output is LOGIC_1 if all inputs are LOGIC_1.
SignalValue evaluate_and(const SignalValue inputs[], int num_inputs) {
    if (num_inputs <= 0) return LOGIC_X; // Undefined for no inputs

    SignalValue has_x = LOGIC_0; // Track if LOGIC_X is encountered
    for (int i = 0; i < num_inputs; i++) {
        if (inputs[i] == LOGIC_0) {
            return LOGIC_0;
        }
        if (inputs[i] == LOGIC_X) {
            has_x = LOGIC_1; // Using LOGIC_1 as a boolean true for has_x
        }
    }
    if (has_x == LOGIC_1) {
        return LOGIC_X;
    }
    return LOGIC_1;
}

// --- NAND gate logic ---
// Inverse of AND.
SignalValue evaluate_nand(const SignalValue inputs[], int num_inputs) {
    SignalValue and_result = evaluate_and(inputs, num_inputs);
    if (and_result == LOGIC_0) return LOGIC_1;
    if (and_result == LOGIC_1) return LOGIC_0;
    return LOGIC_X; // If AND result is X, NAND is also X
}

// --- OR gate logic ---
// Output is LOGIC_1 if any input is LOGIC_1.
// Output is LOGIC_X if any input is LOGIC_X and no input is LOGIC_1.
// Output is LOGIC_0 if all inputs are LOGIC_0.
SignalValue evaluate_or(const SignalValue inputs[], int num_inputs) {
    if (num_inputs <= 0) return LOGIC_X; // Undefined for no inputs

    SignalValue has_x = LOGIC_0;
    for (int i = 0; i < num_inputs; i++) {
        if (inputs[i] == LOGIC_1) {
            return LOGIC_1;
        }
        if (inputs[i] == LOGIC_X) {
            has_x = LOGIC_1;
        }
    }
    if (has_x == LOGIC_1) {
        return LOGIC_X;
    }
    return LOGIC_0;
}

// --- NOR gate logic ---
// Inverse of OR.
SignalValue evaluate_nor(const SignalValue inputs[], int num_inputs) {
    SignalValue or_result = evaluate_or(inputs, num_inputs);
    if (or_result == LOGIC_0) return LOGIC_1;
    if (or_result == LOGIC_1) return LOGIC_0;
    return LOGIC_X; // If OR result is X, NOR is also X
}

// --- XOR2 gate logic ---
// Output is LOGIC_X if any input is LOGIC_X.
// Else, output is LOGIC_1 if inputs are different, LOGIC_0 if same.
SignalValue evaluate_xor2(SignalValue input1, SignalValue input2) {
    if (input1 == LOGIC_X || input2 == LOGIC_X) {
        return LOGIC_X;
    }
    return (input1 != input2) ? LOGIC_1 : LOGIC_0;
}

// --- XNOR2 gate logic ---
// Output is LOGIC_X if any input is LOGIC_X.
// Else, output is LOGIC_1 if inputs are same, LOGIC_0 if different.
SignalValue evaluate_xnor2(SignalValue input1, SignalValue input2) {
    if (input1 == LOGIC_X || input2 == LOGIC_X) {
        return LOGIC_X;
    }
    return (input1 == input2) ? LOGIC_1 : LOGIC_0;
}

// --- NOT1 gate logic ---
SignalValue evaluate_not1(SignalValue input1) {
    if (input1 == LOGIC_0) return LOGIC_1;
    if (input1 == LOGIC_1) return LOGIC_0;
    return LOGIC_X; // If input is X, NOT is also X
}

// --- BUFF1 gate logic ---
SignalValue evaluate_buff1(SignalValue input1) {
    return input1; // Buffer output is the same as input, including X
}
