package src.model;

public class BranchNode extends Node {
    private Node source;
    private int branchNumber;
    
    public BranchNode(int id, String name, Node source, int branchNumber) {
        super(id, name, NodeType.BRNH);
        this.source = source;
        this.branchNumber = branchNumber;
    }
    
    public Node getSource() {
        return source;
    }
    
    public int getBranchNumber() {
        return branchNumber;
    }
    
    @Override
    public String toString() {
        return String.format("%s (ID: %d, Name: %s, Source: %s, Branch: %d)", 
                            type, id, name, source.getName(), branchNumber);
    }
}
