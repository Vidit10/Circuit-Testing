package src.model;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Circuit {
    private String name;
    private List<Node> nodes;
    private List<InputNode> primaryInputs;
    private List<OutputNode> primaryOutputs;
    private Map<String, Node> nodeMap;
    private int nextNodeId;
    
    public Circuit(String name) {
        this.name = name;
        this.nodes = new ArrayList<>();
        this.primaryInputs = new ArrayList<>();
        this.primaryOutputs = new ArrayList<>();
        this.nodeMap = new HashMap<>();
        this.nextNodeId = 1;
    }
    
    public String getName() {
        return name;
    }
    
    public List<Node> getNodes() {
        return nodes;
    }
    
    public List<InputNode> getPrimaryInputs() {
        return primaryInputs;
    }
    
    public List<OutputNode> getPrimaryOutputs() {
        return primaryOutputs;
    }
    
    public Node getNodeByName(String name) {
        return nodeMap.get(name);
    }
    
    public InputNode createInputNode(String name) {
        InputNode node = new InputNode(nextNodeId++, name);
        nodes.add(node);
        primaryInputs.add(node);
        nodeMap.put(name, node);
        return node;
    }
    
    public OutputNode createOutputNode(String name) {
        OutputNode node = new OutputNode(nextNodeId++, name);
        nodes.add(node);
        primaryOutputs.add(node);
        nodeMap.put(name, node);
        return node;
    }
    
    public GateNode createGateNode(String name, GateType gateType) {
        GateNode node = new GateNode(nextNodeId++, name, gateType);
        nodes.add(node);
        nodeMap.put(name, node);
        return node;
    }
    
    public BranchNode createBranchNode(Node source, int branchNumber) {
        String branchName = source.getName() + "_" + branchNumber;
        BranchNode node = new BranchNode(nextNodeId++, branchName, source, branchNumber);
        nodes.add(node);
        nodeMap.put(branchName, node);
        return node;
    }
    
    public void connect(Node source, Node target) {
        source.addOutput(target);
        target.addInput(source);
    }
    
    public void addBranchNodes() {
        List<Node> nodesToProcess = new ArrayList<>(nodes);
        
        for (Node node : nodesToProcess) {
            if (node.getOutputCount() > 1) {
                List<Node> originalOutputs = new ArrayList<>(node.getOutputs());
                node.getOutputs().clear();
                
                for (int i = 0; i < originalOutputs.size(); i++) {
                    Node output = originalOutputs.get(i);
                    BranchNode branchNode = createBranchNode(node, i + 1);
                    
                    // Connect source to branch node
                    connect(node, branchNode);
                    
                    // Connect branch node to original output
                    connect(branchNode, output);
                    
                    // Update the input of the output node
                    output.getInputs().remove(node);
                }
            }
        }
    }
    
    public int getNodeCount() {
        return nodes.size();
    }
    
    public int getInputCount() {
        return primaryInputs.size();
    }
    
    public int getOutputCount() {
        return primaryOutputs.size();
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Circuit: ").append(name).append("\n");
        sb.append("Inputs: ").append(primaryInputs.size()).append("\n");
        sb.append("Outputs: ").append(primaryOutputs.size()).append("\n");
        sb.append("Total Nodes: ").append(nodes.size()).append("\n");
        return sb.toString();
    }
}
