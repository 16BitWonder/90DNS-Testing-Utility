#include <stdio.h>
#include <switch.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "main.h"

PrintConsole *console;

int main(int argc, char **argv)
{
    // Initialize default console
    console = consoleGetDefault();
    consoleInit(console);

    // Configure pad for player 1 accepting input from any controller
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadFullCtrl);
    padInitializeAny(&pad);

    // Initialize sockets
    socketInitializeDefault();
    // Initialize nifm
    nifmInitialize(NifmServiceType_User);

    checkHostnames();

    // Main loop
    while(appletMainLoop())
    {
        // Update the pad inputs
        padUpdate(&pad);

        // Get key pressed on the pad
        u64 kDown = padGetButtonsDown(&pad);

        // Exit on B
        if (kDown & HidNpadButton_B) break;
        // Retry on X
        if (kDown & HidNpadButton_X)
        {
            consoleClear();
            checkHostnames();
        }
    }

    // stop sockets before quit
    socketExit();
    // Quit nifm
    nifmExit();
    consoleExit(console);
    return 0;
}

void checkHostnames() 
{
    printf("90DNS Testing Utility v1.0.4\n\n");

    // Check if system says we're connected to a network (wifi or ethernet)
    Result net_rc = nifmGetInternetConnectionStatus(NULL, NULL, NULL);
    if (R_FAILED(net_rc)) {
        printf(CONSOLE_RED "WARNING, NOT CONNECTED TO ANY NETWORK! AIRPLANE MODE?\n" CONSOLE_RESET);
    }

    printf("Testing:\n");
	
	// Iterate through connection hostnames array
    for (int i = 0; i < sizeof(connectionhostnames)/sizeof(connectionhostnames[0]); i++)
    {
        // Print the connection hostname
        printf("\x1b[2C%s", connectionhostnames[i]);
        // Move the cursor and print progress dots
        printf("\x1b[%dC", 40-console->cursorX);
        printf("...");
        // Update console here so we get a "live" output
        consoleUpdate(console);

        // Resolve the connection hostname
        int result = resolveConnHostname(connectionhostnames[i]);

        // Move the cursor back 3 steps to overwrite the dots and print status
        printf("\x1b[3D");
        switch(result) {
            case DNS_BLOCKED:
                printf(CONSOLE_GREEN "redirected");
                break;
            case DNS_RESOLVED:
                printf(CONSOLE_RED "unredirected");
                break;
            case DNS_UNRESOLVED:
                printf(CONSOLE_YELLOW "unresolved");
                break;
        }

        // Reprint hostname with changed color
        printf("\x1b[%dD%s\n" CONSOLE_RESET, console->cursorX-2, connectionhostnames[i]);

        consoleUpdate(console);
    }

    // Iterate through hostnames array
    for (int i = 0; i < sizeof(hostnames)/sizeof(hostnames[0]); i++)
    {
        // Print the hostname
        printf("\x1b[2C%s", hostnames[i]);
        // Move the cursor and print progress dots
        printf("\x1b[%dC", 40-console->cursorX);
        printf("...");
        // Update console here so we get a "live" output
        consoleUpdate(console);

        // Resolve the hostname
        int result = resolveHostname(hostnames[i]);

        // Move the cursor back 3 steps to overwrite the dots and print status
        printf("\x1b[3D");
        switch(result) {
            case DNS_BLOCKED:
                printf(CONSOLE_GREEN "blocked");
                break;
            case DNS_RESOLVED:
                printf(CONSOLE_RED "unblocked");
                break;
            case DNS_UNRESOLVED:
                printf(CONSOLE_YELLOW "unresolved");
                break;
        }

        // Reprint hostname with changed color
        printf("\x1b[%dD%s\n" CONSOLE_RESET, console->cursorX-2, hostnames[i]);

        consoleUpdate(console);
    }

    printf("\nPress B to exit. Press X to retry.");
    consoleUpdate(console);
}

RESOLVER_STATUS resolveConnHostname(const char* connhostname)
{
    struct hostent *he;
    struct in_addr a;
    // use gethostbyname to attempt connhostname connection
    he = gethostbyname(connhostname);
    if (he)
    {
        // go over all returned addresses for connhostname
        while (*he->h_addr_list)
        {
            bcopy(*he->h_addr_list++, (char *) &a, sizeof(a));
            // succeed if any of them redirect to desired connection address
            if ((strcmp(inet_ntoa(a), CONNECTION_REDIRECT_ADDRESS_USA) == 0) || (strcmp(inet_ntoa(a), CONNECTION_REDIRECT_ADDRESS_FRANCE) == 0) || (strcmp(inet_ntoa(a), CONNECTION_REDIRECT_ADDRESS_SELFHOST) == 0))
            {
                return DNS_BLOCKED;
            }
        }
        return DNS_RESOLVED;
    }
    return DNS_UNRESOLVED;
}

RESOLVER_STATUS resolveHostname(const char* hostname)
{
    struct hostent *he;
    struct in_addr a;
    // use gethostbyname to attempt hostname connection
    he = gethostbyname(hostname);
    if (he)
    {
        // go over all returned addresses for hostname
        while (*he->h_addr_list)
        {
            bcopy(*he->h_addr_list++, (char *) &a, sizeof(a));
            // succeed if any of them redirect to localhost
            if (strcmp(inet_ntoa(a), "127.0.0.1") == 0 || strcmp(inet_ntoa(a), "0.0.0.0") == 0)
            {
                return DNS_BLOCKED;
            }
        }
        return DNS_RESOLVED;
    }
    return DNS_UNRESOLVED;
}
