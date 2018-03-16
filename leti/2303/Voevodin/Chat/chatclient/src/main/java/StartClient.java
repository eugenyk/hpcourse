import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Scanner;

public class StartClient {
    public static void main(String[] args) throws IOException
    {
        NotificationsListener listener = new NotificationsListener()
        {
            @Override
            public void processMessage(String msg)
            {
                System.out.println(msg);
            }

            @Override
            public void processDisconnect()
            {
                System.out.println("serverDisconnect");
            }

            @Override
            public void processConnect()
            {

            }
        };

        System.out.print("Enter the Nickname: ");
        Scanner in = new Scanner(System.in);
        String name = in.nextLine();

        FacadeNioChatClient client = new FacadeNioChatClient(listener);
        client.connect("localhost", 8989, name);
        BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in));
        while (true)
        {
            System.out.print("Message: ");
            String msg = consoleReader.readLine();
            client.send(msg);
        }
    }
}
