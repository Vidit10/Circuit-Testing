package src.parser;

import src.model.*;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
// import java.util.ArrayList;
import java.util.HashMap;
// import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class VerilogParser {
    private static final Pattern MODULE_PATTERN = Pattern.compile("module\\s+(\\w+)\\s*\\(([^;]+)\\);");
    private static final Pattern INPUT_PATTERN = Pattern.compile("input\\s+([^;]+);");
    private static final Pattern OUTPUT_PATTERN = Pattern.compile("output\\s+([^;]+);");
    private static final Pattern WIRE_PATTERN = Pattern.compile("wire\\s+([^;]+);");
    private static final Pattern GATE_PATTERN = Pattern.compile("(\\w+)\\s+(\\w+_\\d+)\\s*\\(([^;]+)\\);");
    
    public Circuit parseVerilogFile(String filePath) throws ParseException {
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            String line;
            StringBuilder content = new StringBuilder();
            
            while ((line = reader.readLine()) != null) {
                // Skip comments and empty lines
                if (line.trim().startsWith("//") || line.trim().isEmpty()) {
                    continue;
                }
                content.append(line).append("\n");
            }
            
            return parseVerilogContent(content.toString());
        } catch (IOException e) {
            throw new ParseException("Error reading Verilog file: " + e.getMessage(), e);
        }
    }
    
    private Circuit parseVerilogContent(String content) throws ParseException {
        // Extract module name and port list
        Matcher moduleMatcher = MODULE_PATTERN.matcher(content);
        if (!moduleMatcher.find()) {
            throw new ParseException("Invalid Verilog format: module declaration not found");
        }
        
        String moduleName = moduleMatcher.group(1);
        // String portList = moduleMatcher.group(2);
        
        Circuit circuit = new Circuit(moduleName);
        
        // Parse input declarations
        Matcher inputMatcher = INPUT_PATTERN.matcher(content);
        if (inputMatcher.find()) {
            String inputDecl = inputMatcher.group(1);
            for (String inputName : inputDecl.split(",")) {
                circuit.createInputNode(inputName.trim());
            }
        }
        
        // Parse output declarations
        Matcher outputMatcher = OUTPUT_PATTERN.matcher(content);
        if (outputMatcher.find()) {
            String outputDecl = outputMatcher.group(1);
            for (String outputName : outputDecl.split(",")) {
                circuit.createOutputNode(outputName.trim());
            }
        }
        
        // Parse wire declarations
        Map<String, Node> wireNodes = new HashMap<>();
        Matcher wireMatcher = WIRE_PATTERN.matcher(content);
        if (wireMatcher.find()) {
            String wireDecl = wireMatcher.group(1);
            for (String wireName : wireDecl.split(",")) {
                wireName = wireName.trim();
                // Create a placeholder node for the wire
                GateNode wireNode = circuit.createGateNode(wireName, null);
                wireNodes.put(wireName, wireNode);
            }
        }
        
        // Parse gate declarations
        Matcher gateMatcher = GATE_PATTERN.matcher(content);
        while (gateMatcher.find()) {
            String gateType = gateMatcher.group(1);
            String gateName = gateMatcher.group(2);
            String connections = gateMatcher.group(3);
            
            // Parse connections
            String[] connectionList = connections.split(",");
            String outputName = connectionList[0].trim();
            
            // Create or get the output node
            Node outputNode = circuit.getNodeByName(outputName);
            if (outputNode == null) {
                // This might be a wire that wasn't declared
                outputNode = circuit.createGateNode(outputName, null);
            }
            
            // Create the gate node
            GateNode gateNode = circuit.createGateNode(gateName, parseGateType(gateType));
            
            // Connect gate to output
            circuit.connect(gateNode, outputNode);
            
            // Connect inputs to gate
            for (int i = 1; i < connectionList.length; i++) {
                String inputName = connectionList[i].trim();
                Node inputNode = circuit.getNodeByName(inputName);
                
                if (inputNode == null) {
                    throw new ParseException("Undefined node: " + inputName);
                }
                
                circuit.connect(inputNode, gateNode);
            }
        }
        
        // Add branch nodes for fanouts
        circuit.addBranchNodes();
        
        return circuit;
    }
    
    private GateType parseGateType(String typeStr) throws ParseException {
        try {
            return GateType.valueOf(typeStr.toUpperCase());
        } catch (IllegalArgumentException e) {
            throw new ParseException("Unknown gate type: " + typeStr);
        }
    }
}
