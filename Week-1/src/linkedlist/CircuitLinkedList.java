package src.linkedlist;

import src.model.Circuit;
import src.model.Node;
import src.model.NodeType;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class CircuitLinkedList {
    private Circuit circuit;
    private List<NodeEntry> nodeEntries;
    private Map<Integer, Integer> nodeIdToIndex;
    
    public CircuitLinkedList(Circuit circuit) {
        this.circuit = circuit;
        this.nodeEntries = new ArrayList<>();
        this.nodeIdToIndex = new HashMap<>();
        
        // Convert circuit to linked list representation
        convertCircuitToLinkedList();
    }
    
    private void convertCircuitToLinkedList() {
        // First, create entries for all nodes
        for (Node node : circuit.getNodes()) {
            NodeEntry entry = new NodeEntry(node);
            int index = nodeEntries.size();
            nodeEntries.add(entry);
            nodeIdToIndex.put(node.getId(), index);
        }
    }
    
    public List<NodeEntry> getNodeEntries() {
        return nodeEntries;
    }
    
    public NodeEntry getEntryByNodeId(int nodeId) {
        Integer index = nodeIdToIndex.get(nodeId);
        if (index != null) {
            return nodeEntries.get(index);
        }
        return null;
    }
    
    public List<NodeEntry> getPrimaryInputEntries() {
        List<NodeEntry> piEntries = new ArrayList<>();
        for (NodeEntry entry : nodeEntries) {
            if (entry.getNode().getType() == NodeType.PI) {
                piEntries.add(entry);
            }
        }
        return piEntries;
    }
    
    public List<NodeEntry> getPrimaryOutputEntries() {
        List<NodeEntry> poEntries = new ArrayList<>();
        for (NodeEntry entry : nodeEntries) {
            if (entry.getNode().getType() == NodeType.PO) {
                poEntries.add(entry);
            }
        }
        return poEntries;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Circuit Linked List: ").append(circuit.getName()).append("\n");
        
        for (NodeEntry entry : nodeEntries) {
            sb.append(entry.toString()).append("\n");
        }
        
        return sb.toString();
    }
    
    public void printLinkedListFormat() {
        System.out.println("Linked List Representation for " + circuit.getName() + ":");
        System.out.println("Format: ID: Name (Type) Fanins: [ids] Fanouts: [ids]");
        System.out.println("----------------------------------------------------");
        
        for (NodeEntry entry : nodeEntries) {
            System.out.println(entry);
        }
    }
}
