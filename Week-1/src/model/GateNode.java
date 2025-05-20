package src.model;

public class GateNode extends Node {
    private GateType gateType;
    
    public GateNode(int id, String name, GateType gateType) {
        super(id, name, NodeType.GATE);
        this.gateType = gateType;
    }
    
    public GateType getGateType() {
        return gateType;
    }
    
    @Override
    public String toString() {
        return String.format("%s (ID: %d, Name: %s, Type: %s)", type, id, name, gateType);
    }
}
