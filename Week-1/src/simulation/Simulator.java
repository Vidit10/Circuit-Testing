package src.simulation;

import src.linkedlist.CircuitLinkedList;
import src.model.Circuit;
import src.parser.ParseException;
import src.parser.VerilogParser;

public class Simulator {
    private Circuit circuit;
    private CircuitLinkedList circuitList;
    
    public Simulator() {
        this.circuit = null;
        this.circuitList = null;
    }
    
    public void loadVerilogFile(String filePath) throws ParseException {
        VerilogParser parser = new VerilogParser();
        circuit = parser.parseVerilogFile(filePath);
        circuitList = new CircuitLinkedList(circuit);
        
        System.out.println("Loaded circuit: " + circuit.getName());
        System.out.println("Primary inputs: " + circuit.getPrimaryInputs().size());
        System.out.println("Primary outputs: " + circuit.getPrimaryOutputs().size());
        System.out.println("Total nodes: " + circuit.getNodes().size());
    }
    
    public Circuit getCircuit() {
        return circuit;
    }
    
    public CircuitLinkedList getCircuitList() {
        return circuitList;
    }
    
    public int countPaths() {
        if (circuitList == null) {
            throw new IllegalStateException("No circuit loaded");
        }
        
        PathCounter pathCounter = new PathCounter(circuitList);
        return pathCounter.countPaths();
    }
    
    public void printCircuitInfo() {
        if (circuit == null) {
            System.out.println("No circuit loaded");
            return;
        }
        
        System.out.println(circuit);
    }
    
    public void printLinkedListRepresentation() {
        if (circuitList == null) {
            System.out.println("No circuit loaded");
            return;
        }
        
        circuitList.printLinkedListFormat();
    }
    
    public void printPathCounts() {
        if (circuitList == null) {
            System.out.println("No circuit loaded");
            return;
        }
        
        PathCounter pathCounter = new PathCounter(circuitList);
        pathCounter.countPaths();
        pathCounter.printPathCounts();
    }
}
