// NodeEntry.java
package src.linkedlist;

import src.model.Node;
// import src.model.NodeType;

import java.util.ArrayList;
import java.util.List;

public class NodeEntry {
    private Node node;
    private List<Integer> fanins;  // IDs of nodes that are inputs to this node
    private List<Integer> fanouts; // IDs of nodes that are outputs of this node
    
    public NodeEntry(Node node) {
        this.node = node;
        this.fanins = new ArrayList<>();
        this.fanouts = new ArrayList<>();
        
        // Initialize fanins and fanouts from the node
        for (Node input : node.getInputs()) {
            fanins.add(input.getId());
        }
        
        for (Node output : node.getOutputs()) {
            fanouts.add(output.getId());
        }
    }
    
    public Node getNode() {
        return node;
    }
    
    public List<Integer> getFanins() {
        return fanins;
    }
    
    public List<Integer> getFanouts() {
        return fanouts;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(node.getId()).append(": ");
        sb.append(node.getName()).append(" (").append(node.getType()).append(")");
        
        sb.append(" Fanins: [");
        for (int i = 0; i < fanins.size(); i++) {
            if (i > 0) sb.append(", ");
            sb.append(fanins.get(i));
        }
        sb.append("]");
        
        sb.append(" Fanouts: [");
        for (int i = 0; i < fanouts.size(); i++) {
            if (i > 0) sb.append(", ");
            sb.append(fanouts.get(i));
        }
        sb.append("]");
        
        return sb.toString();
    }
}
