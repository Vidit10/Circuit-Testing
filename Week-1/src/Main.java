package src;

import src.parser.ParseException;
import src.simulation.Simulator;
// import src.util.FileUtils;

// import java.util.List;
import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        Simulator simulator = new Simulator();
        
        System.out.println("Verilog Circuit Simulator");
        System.out.println("=========================");
        
        boolean running = true;
        while (running) {
            System.out.println("\nOptions:");
            System.out.println("1. Load Verilog file");
            System.out.println("2. Show circuit information");
            System.out.println("3. Show linked list representation");
            System.out.println("4. Count paths in circuit");
            System.out.println("5. Exit");
            System.out.print("Enter choice: ");
            
            int choice = scanner.nextInt();
            scanner.nextLine(); // Consume newline
            
            switch (choice) {
                case 1:
                    loadVerilogFile(scanner, simulator);
                    break;
                case 2:
                    simulator.printCircuitInfo();
                    break;
                case 3:
                    simulator.printLinkedListRepresentation();
                    break;
                case 4:
                    simulator.printPathCounts();
                    break;
                case 5:
                    running = false;
                    break;
                default:
                    System.out.println("Invalid choice");
            }
        }
        
        System.out.println("Exiting simulator");
        scanner.close();
    }
    
    private static void loadVerilogFile(Scanner scanner, Simulator simulator) {
        System.out.print("Enter Verilog file path: ");
        String filePath = scanner.nextLine();
        
        try {
            simulator.loadVerilogFile(filePath);
            System.out.println("File loaded successfully");
        } catch (ParseException e) {
            System.out.println("Error loading file: " + e.getMessage());
        }
    }
}
