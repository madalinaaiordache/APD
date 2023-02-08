import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class Tema2 {
    static int numThreads;
    static ExecutorService executor;
    static File ordersInputFile;
    static File productsInputFile;
    static FileWriter ordersOutputFile;
    static FileWriter productsOutputFile;
    static HashMap<String, AtomicInteger> orders = new HashMap<>();

    public static void main(String[] args) throws IOException, InterruptedException {
        String inputDir = args[0];
        numThreads = Integer.parseInt(args[1]);

        ordersInputFile = new File(inputDir, "orders.txt");
        productsInputFile = new File(inputDir, "order_products.txt");

        ordersOutputFile = new FileWriter("orders_out.txt");
        productsOutputFile = new FileWriter("order_products_out.txt");

        LinkedList<Thread> threads = new LinkedList();
        List<ProductsReader> awaiting = Collections.synchronizedList(new ArrayList<>());

        for (int i = 0; i < numThreads; i++) {
            Thread thread = new Thread(new OrdersReader(i, awaiting));
            threads.add(thread);
            thread.start();
        }

        // Wait order readers to finish
        for (Thread thread:threads) {
            thread.join();
        }

        executor = Executors.newFixedThreadPool(numThreads);

        // Add product workers to thread pool
        for (ProductsReader productReader : awaiting) {
            executor.execute(productReader);
        }

        // Stop receiving task in threadpool
        executor.shutdown();
        executor.awaitTermination(100, TimeUnit.SECONDS);

        // Flush files
        ordersOutputFile.close();
        productsOutputFile.close();
    }
}