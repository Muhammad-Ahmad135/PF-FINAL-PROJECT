#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TOTAL_SEATS 30
#define TOTAL_STOPS 5
#define PASSWORD "mybus"
#define BASE_FARE 10.0
#define DISCOUNT 0.1

typedef struct {
    char cnic[20];
    int age;
    char entryTime[20];
    char exitTime[20];
    int entryStop;
    int exitStop;
} Passenger;

int* seats; 
Passenger* passengers; 
char** stops; 

void getCurrentTime(char* buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
}

int findFreeSeat() {
    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (seats[i] == 0) {
            return i;
        }
    }
    return -1;
}

void entry() {
    int entryStop;
    Passenger p;

    printf("Enter entry stop (0-4): ");
    scanf("%d", &entryStop);
    printf("Enter CNIC: ");
    scanf("%s", p.cnic);
    printf("Enter age: ");
    scanf("%d", &p.age);
}
    int seat = findFreeSeat();
    if (seat == -1) {
        printf("No free seats available!\n");
        return;
    }

    p.entryStop = entryStop;
    getCurrentTime(p.entryTime);
    passengers[seat] = p;
    seats[seat] = 1;
    printf("Passenger allocated to seat %d\n", seat);
}

void exitPassenger() {
    int exitStop, seat;
    printf("Enter exiting stop (0-4): ");
    scanf("%d", &exitStop);
    printf("Enter seat number: ");
    scanf("%d", &seat);

    if (seat < 0 || seat >= TOTAL_SEATS || seats[seat] == 0) {
        printf("Invalid or empty seat!\n");
        return;
    }

    passengers[seat].exitStop = exitStop;
    getCurrentTime(passengers[seat].exitTime);

    int stopsTraveled = abs(passengers[seat].exitStop - passengers[seat].entryStop);
    double fare = stopsTraveled * BASE_FARE;
    if (passengers[seat].age < 10 || passengers[seat].age > 60) {
        fare -= fare * DISCOUNT;
    }

    printf("Fare: $%.2f\n", fare);
    seats[seat] = 0; 

void checkRecords() {
    char password[20];
    printf("Enter password: ");
    scanf("%s", password);

    if (strcmp(password, PASSWORD) != 0) {
        printf("Incorrect password!\n");
        return;
    }

    int found = 0; 
    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (passengers[i].entryStop >= 0) {
            found = 1;
            printf("Seat %d: %s\n", i, seats[i] == 1 ? "Occupied" : "Exited");
            printf("  CNIC: %s\n", passengers[i].cnic);
            printf("  Age: %d\n", passengers[i].age);
            printf("  Entry Stop: %d (%s)\n", passengers[i].entryStop, stops[passengers[i].entryStop]);
            printf("  Entry Time: %s\n", passengers[i].entryTime);

            if (passengers[i].exitStop > 0) {
                printf("  Exit Stop: %d (%s)\n", passengers[i].exitStop, stops[passengers[i].exitStop]);
                printf("  Exit Time: %s\n", passengers[i].exitTime);
            } else {
                printf("  Exit Stop: Not exited yet\n");
                printf("  Exit Time: Not available\n");
            }
        }
    }

    if (!found) {
        printf("No active records found.\n");
    }
}

void initialize() {
    seats = (int*)malloc(TOTAL_SEATS * sizeof(int));
    passengers = (Passenger*)malloc(TOTAL_SEATS * sizeof(Passenger));
    stops = (char**)malloc(TOTAL_STOPS * sizeof(char*));
    for (int i = 0; i < TOTAL_STOPS; i++) {
        stops[i] = (char*)malloc(20 * sizeof(char)); 
    }

    for (int i = 0; i < TOTAL_SEATS; i++) {
        seats[i] = 0; 
        passengers[i].entryStop = -1; 
    }
    strcpy(stops[0], "Stop1");
    strcpy(stops[1], "Stop2");
    strcpy(stops[2], "Stop3");
    strcpy(stops[3], "Stop4");
    strcpy(stops[4], "Stop5");
}

void cleanup() {
    free(seats);
    free(passengers);
    for (int i = 0; i < TOTAL_STOPS; i++) {
        free(stops[i]);
    }
    free(stops);
}

int main() {
    initialize();

    int choice;
    while (1) {
        printf("\n1. Entry\n2. Exit\n3. Check Records\n4. Exit Program\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                entry();
                break;
            case 2:
                exitPassenger();
                break;
            case 3:
                checkRecords();
                break;
            case 4:
                printf("Exiting program and clearing all records...\n");
                cleanup();
                exit(0);
            default:
                printf("Invalid option!\n");
        }
    }

    return 0;
}
