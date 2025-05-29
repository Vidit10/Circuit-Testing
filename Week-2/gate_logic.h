#ifndef GATE_LOGIC_H
#define GATE_LOGIC_H

// Represents the possible logic values of a signal
typedef enum
{
    LOGIC_0 = 0,
    LOGIC_1 = 1,
    LOGIC_X = 2 // Represents an unknown or uninitialized state
} SignalValue;

// --- Gate Logic Evaluation Functions ---

/**
 * @brief Converts a SignalValue enum to its character representation.
 * @param val The SignalValue.
 * @return Character '0', '1', or 'X'.
 */
char signal_value_to_char(SignalValue val);

/**
 * @brief Evaluates an N-input AND gate.
 * @param inputs Array of input signal values.
 * @param num_inputs Number of inputs (e.g., 2 for AND2, 3 for AND3, etc.).
 * @return Output SignalValue.
 */
SignalValue evaluate_and(const SignalValue inputs[], int num_inputs);

/**
 * @brief Evaluates an N-input NAND gate.
 * @param inputs Array of input signal values.
 * @param num_inputs Number of inputs.
 * @return Output SignalValue.
 */
SignalValue evaluate_nand(const SignalValue inputs[], int num_inputs);

/**
 * @brief Evaluates an N-input OR gate.
 * @param inputs Array of input signal values.
 * @param num_inputs Number of inputs.
 * @return Output SignalValue.
 */
SignalValue evaluate_or(const SignalValue inputs[], int num_inputs);

/**
 * @brief Evaluates an N-input NOR gate.
 * @param inputs Array of input signal values.
 * @param num_inputs Number of inputs.
 * @return Output SignalValue.
 */
SignalValue evaluate_nor(const SignalValue inputs[], int num_inputs);

/**
 * @brief Evaluates a 2-input XOR gate (XOR2).
 * @param input1 First input signal value.
 * @param input2 Second input signal value.
 * @return Output SignalValue.
 */
SignalValue evaluate_xor2(SignalValue input1, SignalValue input2);

/**
 * @brief Evaluates a 2-input XNOR gate (XNOR2).
 * @param input1 First input signal value.
 * @param input2 Second input signal value.
 * @return Output SignalValue.
 */
SignalValue evaluate_xnor2(SignalValue input1, SignalValue input2);

/**
 * @brief Evaluates a 1-input NOT gate (NOT1).
 * @param input1 Input signal value.
 * @return Output SignalValue.
 */
SignalValue evaluate_not1(SignalValue input1);

/**
 * @brief Evaluates a 1-input Buffer (BUFF1).
 * @param input1 Input signal value.
 * @return Output SignalValue.
 */
SignalValue evaluate_buff1(SignalValue input1);

#endif // GATE_LOGIC_H
