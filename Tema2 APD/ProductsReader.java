import com.sun.tools.javac.Main;

import java.io.*;
import java.util.Scanner;
import java.util.concurrent.BrokenBarrierException;

public class ProductsReader implements Runnable{
    int id;
    int numProducts;
    String orderId;

    public ProductsReader(int id, int numProducts, String orderId) {
        this.id = id;
        this.numProducts = numProducts;
        this.orderId = orderId;
    }

    @Override
    public void run() {
        try {
            File ordersFile = Tema2.productsInputFile;
            BufferedReader bufferedReader = new BufferedReader(new FileReader(ordersFile));
            int index = 0;

            String line;
            while ((line = bufferedReader.readLine()) != null) {
                if (index % Tema2.numThreads == id) {
                    String[] args = line.split(",");
                    if (args[0].equals(orderId)) {
                        // Lock the file
                        synchronized (Tema2.productsOutputFile) {
                            Tema2.productsOutputFile.write(orderId + "," + args[1] + ",shipped\n");
                            if (Tema2.orders.get(orderId).decrementAndGet() == 0) {
                                Tema2.ordersOutputFile.write(orderId + "," + numProducts + ",shipped\n");
                            }
                        }
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
