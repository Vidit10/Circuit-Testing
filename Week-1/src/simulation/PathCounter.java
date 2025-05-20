package src.simulation;

import src.linkedlist.CircuitLinkedList;
import src.linkedlist.NodeEntry;
import src.model.NodeType;

import java.util.*;

public class PathCounter {
    private CircuitLinkedList circuitList;
    private Map<Integer, Integer> pathCounts;
    private Map<Integer, Boolean> visited;
    private List<Integer> topologicalOrder;
    
    public PathCounter(CircuitLinkedList circuitList) {
        this.circuitList = circuitList;
        this.pathCounts = new HashMap<>();
        this.visited = new HashMap<>();
        this.topologicalOrder = new ArrayList<>();
    }
    
    public int countPaths() {
        // Initialize path counts
        for (NodeEntry entry : circuitList.getNodeEntries()) {
            int nodeId = entry.getNode().getId();
            pathCounts.put(nodeId, 0);
            visited.put(nodeId, false);
        }
        
        // Set path count of primary inputs to 1
        for (NodeEntry entry : circuitList.getPrimaryInputEntries()) {
            int nodeId = entry.getNode().getId();
            pathCounts.put(nodeId, 1);
            visited.put(nodeId, true);
        }
        
        // Perform topological sort
        performTopologicalSort();
        
        // Calculate path counts using topological order
        for (int nodeId : topologicalOrder) {
            NodeEntry entry = circuitList.getEntryByNodeId(nodeId);
            
            if (entry.getNode().getType() != NodeType.PI) {
                int sum = 0;
                for (int inputId : entry.getFanins()) {
                    sum += pathCounts.get(inputId);
                }
                pathCounts.put(nodeId, sum);
            }
        }
        
        // Sum up path counts for primary outputs
        int totalPaths = 0;
        for (NodeEntry entry : circuitList.getPrimaryOutputEntries()) {
            totalPaths += pathCounts.get(entry.getNode().getId());
        }
        
        return totalPaths;
    }
    
    private void performTopologicalSort() {
        // Reset visited flags
        for (Integer nodeId : visited.keySet()) {
            visited.put(nodeId, false);
        }
        
        topologicalOrder.clear();
        
        // Start DFS from primary inputs
        for (NodeEntry entry : circuitList.getPrimaryInputEntries()) {
            if (!visited.get(entry.getNode().getId())) {
                topologicalSortDFS(entry.getNode().getId());
            }
        }
        
        // Reverse the order to get correct topological sort
        Collections.reverse(topologicalOrder);
    }
    
    private void topologicalSortDFS(int nodeId) {
        visited.put(nodeId, true);
        
        NodeEntry entry = circuitList.getEntryByNodeId(nodeId);
        for (int outputId : entry.getFanouts()) {
            if (!visited.get(outputId)) {
                topologicalSortDFS(outputId);
            }
        }
        
        topologicalOrder.add(nodeId);
    }
    
    public Map<Integer, Integer> getPathCounts() {
        return pathCounts;
    }
    
    public void printPathCounts() {
        System.out.println("Path Counts:");
        System.out.println("------------");
        
        for (NodeEntry entry : circuitList.getNodeEntries()) {
            int nodeId = entry.getNode().getId();
            System.out.printf("Node %d (%s): %d paths\n", 
                             nodeId, entry.getNode().getName(), pathCounts.get(nodeId));
        }
        
        int totalPaths = 0;
        for (NodeEntry entry : circuitList.getPrimaryOutputEntries()) {
            totalPaths += pathCounts.get(entry.getNode().getId());
        }
        
        System.out.println("------------");
        System.out.println("Total paths in circuit: " + totalPaths);
    }
}
