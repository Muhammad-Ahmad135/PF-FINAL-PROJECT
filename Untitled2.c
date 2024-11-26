#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PASSENGERS 400         // 10 buses * 40 capacity
#define MAX_BUSES 10
#define SEATS_PER_BUS 40
#define MAX_STOPS 5
#define CUSTOMER_ID_MIN 1000
#define CUSTOMER_ID_MAX 9999
#define DISCOUNT_HEIGHT_THRESHOLD 3.0     // height above 3ft
#define BASE_FARE 20.0
#define ADDITIONAL_STOP_FARE 10.0
#define PEAK_HOUR_START 17               // 5 PM
#define PEAK_HOUR_END 20                // 8 PM

typedef struct {
    char name[50];
    int age;
    float height;       // Height in feet
    char paymentMethod[10];
    char endingStop[20];
    int busNumber;
    int seatNumber;
    int customerID;
    float distance;
    float fare;
    char bookingTime[50];
    char discountDetails[200];        // Stores discount details
    double cnic;
} Passenger;

typedef struct {
    char name[50];
    int tripsCompleted;
    float totalDistance;
    float totalFarePaid;
} UserProfile;

// Global variables
int seats[MAX_BUSES][SEATS_PER_BUS] = {0}; // 0 = vacant, 1 = booked
int passengerCount = 0;

const char *stops[MAX_STOPS] = {"Stop A", "Stop B", "Stop C", "Stop D", "Stop E"};

// Function prototypes
void displayMenu();
void bookTicket(int stopIndex);
void saveToFile(Passenger passenger);
void adminPanel();
void viewDailyRecords();
void resetDailyRecords();
void viewLoyaltyProfiles();
void updateUserProfile(UserProfile *user, float distance, float fare);
void saveUserProfile(UserProfile user);
void displayStops();
int generateCustomerID();
void getNewCustomerDetails(Passenger *customer, int stopIndex);
float applyDiscount(float fare, float height, char paymentMethod[], int numberOfStops, char* discountDetails);
float applyPeakHourCharge(float fare);
void processPassengersAtStop(int stopIndex);

// Function Definitions

void displayMenu() {
    printf("\nWelcome to XYZ Public Transport System\n");
    printf("1. Start Boarding\n");
    printf("2. Admin Panel\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
}

void displayStops() {
    printf("\nAvailable Stops:\n");
    for (int i = 0; i < MAX_STOPS; i++) {
        printf("%d. %s\n", i + 1, stops[i]);
    }
}

void bookTicket(int stopIndex) {
    Passenger passenger;
 time_t t;
    // Generate new customer ID and prompt for new customer details
    passenger.customerID = generateCustomerID();
    printf("Your new customer ID is: %d\n", passenger.customerID);

    // Get new customer details
    getNewCustomerDetails(&passenger, stopIndex);

    // Get ending stop
    int endChoice;
    do {
        printf("Select the ending stop (greater than %d to %d): ", stopIndex + 1, MAX_STOPS);
        scanf("%d", &endChoice);

        if (endChoice <= stopIndex) {
            printf("Invalid stop selection. The ending stop must be greater than the current stop.\n");
        }
    } while (endChoice <= stopIndex);

    strcpy(passenger.endingStop, stops[endChoice - 1]);

    // Calculate the number of stops
    int numberOfStops = endChoice - stopIndex;
    passenger.distance = numberOfStops * ADDITIONAL_STOP_FARE;  // 10 per stop after the first
    passenger.fare = BASE_FARE + (numberOfStops - 1) * ADDITIONAL_STOP_FARE;  // Starting fare $20

    // Apply discounts based on the height, payment method, and number of stops
    passenger.fare = applyDiscount(passenger.fare, passenger.height, passenger.paymentMethod, numberOfStops, passenger.discountDetails);

    // Apply peak hour charge if applicable
    passenger.fare = applyPeakHourCharge(passenger.fare);

    // Assign seat and bus
    for (int i = 0; i < MAX_BUSES; i++) {
        for (int j = 0; j < SEATS_PER_BUS; j++) {
            if (seats[i][j] == 0) { // Vacant seat
                passenger.busNumber = i + 1;
                passenger.seatNumber = j + 1;
                seats[i][j] = 1; // Mark seat as booked
                goto SeatAssigned;
            }
        }
    }
    printf("All seats are full!\n");
    return;

SeatAssigned:
    // Generate booking time
   
    time(&t);
    strcpy(passenger.bookingTime, ctime(&t));
    passenger.bookingTime[strcspn(passenger.bookingTime, "\n")] = '\0'; // Remove newline

    // Save to file
    saveToFile(passenger);

    // Update loyalty profile
    UserProfile user;
    strcpy(user.name, passenger.name);
    updateUserProfile(&user, passenger.distance, passenger.fare);

    // Display booking details and discount information
    printf("\nBooking Successful!\n");
    printf("Name: %s\nBus Number: %d\nSeat Number: %d\nCustomer ID: %d\n", 
           passenger.name, passenger.busNumber, passenger.seatNumber, passenger.customerID);
    printf("Booking Time: %s\nEstimated Fare: $%.2f\n", passenger.bookingTime, passenger.fare);
    printf("Discounts Applied: %s\n", passenger.discountDetails);
}

void saveToFile(Passenger passenger) {
    FILE *file = fopen("passenger_records.txt", "a");
    
    if (!file) {
        printf("Error saving to file.\n");
        return;
    }

    // Write passenger data
    fprintf(file, "%-20s%-5d%-8.2f%-10s%-15s%-15s%-10d%-10d%-10d%-10.2f%-10.2f%-15s%-15s\n", 
            passenger.name, passenger.age, passenger.height, passenger.paymentMethod,
            stops[0], passenger.endingStop, passenger.busNumber,
            passenger.seatNumber, passenger.customerID, passenger.distance, passenger.fare, passenger.bookingTime, passenger.discountDetails);

    fclose(file);
}

int generateCustomerID() {
    // Generate a random customer ID between 1000 and 9999
    return (rand() % (CUSTOMER_ID_MAX - CUSTOMER_ID_MIN + 1)) + CUSTOMER_ID_MIN;
}

void getNewCustomerDetails(Passenger *customer, int stopIndex) {
    printf("Enter your name: ");
    scanf(" %[^\n]", customer->name); // Using this format specifier to read the entire name with spaces

    printf("Enter your age: ");
    scanf("%d", &customer->age);
    
    printf("Enter your height (in feet): ");
    scanf("%f", &customer->height);

    printf("Enter your payment method (cash/card): ");
    scanf("%s", customer->paymentMethod);
}

float applyDiscount(float fare, float height, char paymentMethod[], int numberOfStops, char* discountDetails) {
    int discountApplied = 0;

    // Apply a 3% discount if the payment method is a card
    if (strcmp(paymentMethod, "card") == 0) {
        fare *= 0.97; // Apply 3% discount
        strcat(discountDetails, "Card payment (3%)");
        discountApplied = 1;
    }

    // Apply a 3% discount if traveling more than 2 stops
    if (numberOfStops > 2) {
        fare *= 0.97; // Apply 3% discount
        if (discountApplied) strcat(discountDetails, ", ");
        strcat(discountDetails, "More than 2 stops (3%)");
        discountApplied = 1;
    }

    // Apply a 3% discount if the height is less than 3 feet
    if (height < DISCOUNT_HEIGHT_THRESHOLD) {
        fare *= 0.97; // Apply 3% discount
        if (discountApplied) strcat(discountDetails, ", ");
        strcat(discountDetails, "Height less than 3 feet (3%)");
    }

    return fare;
}

float applyPeakHourCharge(float fare) {
    time_t t;
    struct tm *tm_info;
    time(&t);
    tm_info = localtime(&t);

    // Check if the time is between 5 PM and 8 PM
    if (tm_info->tm_hour >= PEAK_HOUR_START && tm_info->tm_hour < PEAK_HOUR_END) {
        fare *= 1.05; // Apply a 5% peak hour surcharge
        printf("Peak hour charge applied (5%%)\n");
    }

    return fare;
}

void updateUserProfile(UserProfile *user, float distance, float fare) {
    user->tripsCompleted++;
    user->totalDistance += distance;
    user->totalFarePaid += fare;

    saveUserProfile(*user);
}

void saveUserProfile(UserProfile user) {
    FILE *file = fopen("loyalty_data.txt", "a");
    if (!file) {
        printf("Error saving loyalty profile.\n");
        return;
    }

    // Add user profile data
    fprintf(file, "%-20s%-15d%-15.2f%-15.2f\n", user.name, user.tripsCompleted, user.totalDistance, user.totalFarePaid);

    fclose(file);
}

void processPassengersAtStop(int stopIndex) {
    char choice;
    do {
        printf("\nNow boarding passengers at %s.\n", stops[stopIndex]);
        printf("Do you want to book a ticket for this stop? (y/n): ");
        scanf(" %c", &choice);

        if (choice == 'y' || choice == 'Y') {
            bookTicket(stopIndex);
        } else {
            printf("Finished boarding at %s. Moving to the next stop.\n", stops[stopIndex]);
        }
    } while (choice == 'y' || choice == 'Y');
}

void adminPanel() {
    char password[20];
    printf("Enter admin password: ");
    scanf("%s", password);

    if (strcmp(password, "admin123") == 0) {
        int choice;
        printf("\nAdmin Panel\n");
        printf("1. View Daily Records\n");
        printf("2. Reset Records\n");
        printf("3. View Loyalty Profiles\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                viewDailyRecords();
                break;
            case 2:
                resetDailyRecords();
                break;
            case 3:
                viewLoyaltyProfiles();
                break;
            default:
                printf("Invalid choice.\n");
        }
    } else {
        printf("Invalid password!\n");
    }
}

void viewDailyRecords() {
    FILE *file = fopen("passenger_records.txt", "r");
    if (!file) {
        printf("No records found.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
}

void resetDailyRecords() {
    FILE *file = fopen("passenger_records.txt", "w");
    if (!file) {
        printf("Error resetting records.\n");
        return;
    }
    fclose(file);
    printf("Daily records reset successfully.\n");
}

void viewLoyaltyProfiles() {
    FILE *file = fopen("loyalty_data.txt", "r");
    if (!file) {
        printf("No loyalty profiles found.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    int choice;
    do {
        displayMenu();
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                for (int i = 0; i < MAX_STOPS; i++) {
                    processPassengersAtStop(i);
                }
                break;
            case 2:
                adminPanel();
                break;
            case 3:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 3);

    return 0;
}
