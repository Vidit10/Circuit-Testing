package src.model;

import java.util.ArrayList;
import java.util.List;

public abstract class Node {
    protected int id;
    protected String name;
    protected NodeType type;
    protected List<Node> inputs;
    protected List<Node> outputs;
    
    public Node(int id, String name, NodeType type) {
        this.id = id;
        this.name = name;
        this.type = type;
        this.inputs = new ArrayList<>();
        this.outputs = new ArrayList<>();
    }
    
    public void addInput(Node node) {
        if (!inputs.contains(node)) {
            inputs.add(node);
        }
    }
    
    public void addOutput(Node node) {
        if (!outputs.contains(node)) {
            outputs.add(node);
        }
    }
    
    public int getId() {
        return id;
    }
    
    public String getName() {
        return name;
    }
    
    public NodeType getType() {
        return type;
    }
    
    public List<Node> getInputs() {
        return inputs;
    }
    
    public List<Node> getOutputs() {
        return outputs;
    }
    
    public int getInputCount() {
        return inputs.size();
    }
    
    public int getOutputCount() {
        return outputs.size();
    }
    
    @Override
    public String toString() {
        return String.format("%s (ID: %d, Name: %s)", type, id, name);
    }
}
