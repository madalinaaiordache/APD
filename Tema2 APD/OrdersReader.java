import java.io.*;
import java.util.LinkedList;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicInteger;

public class OrdersReader implements Runnable {
    int id;
    List<ProductsReader> awaiting;

    public OrdersReader(int id, List<ProductsReader> awaiting) {
        this.id = id;
        this.awaiting = awaiting;
    }

    @Override
    public void run() {
        try {
            File ordersFile = Tema2.ordersInputFile;
            BufferedReader bufferedReader = new BufferedReader(new FileReader(ordersFile));
            int index = 0;

            String line;
            while ((line = bufferedReader.readLine()) != null) {
                // Filter Assigned Lines
                if (index % Tema2.numThreads == id) {
                    String[] args = line.split(",");
                    int numProducts = Integer.parseInt(args[1]);

                    // Skip empty orders
                    if (numProducts == 0) {
                        index++;
                        continue;
                    }

                    // Lock for hashmap
                    synchronized (Tema2.orders) {
                        Tema2.orders.put(args[0], new AtomicInteger(numProducts));
                    }

                    // Add Product readers to the list, to run on the threadpool after all orders are read
                    for (int i = 0; i < Tema2.numThreads; i++) {
                        awaiting.add(new ProductsReader(i, numProducts, args[0]));
                    }
                }
                index++;
            }
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
