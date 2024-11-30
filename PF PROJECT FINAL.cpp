#include <stdio.h>        //to include functions like printf,scanf,fopen,fclose,fgets etc
#include <stdlib.h>      //for random id generating ;rand(),srand()
#include <string.h>     //to include string functions;strcmp,strcpy,strlen
#include <time.h>      //to include real time in booking; time()
#define MAX_PASSENGERS 400         // 10 buses * 40 capacity=400 passegers
#define MAX_BUSES 10               //total buses of system
#define SEATS_PER_BUS 40           //no of seats in each bus
#define MAX_STOPS 5                //the bus travels through 5 stops
#define CUSTOMER_ID_MIN 1000       //customer id is is a 4-digit id 
#define CUSTOMER_ID_MAX 9999       
#define DISCOUNT_HEIGHT 3.0         // height above 3ft
#define BASE_FARE 20.0              //base fare for ride(same for everyone)
#define ADDITIONAL_STOP_FARE 10.0   //if stops>1 10$ are increase for each stop + base fare
#define PEAK_HOUR_START 17               //using 24hr zone 5 PM
#define PEAK_HOUR_END 20                // 8 PM

typedef struct {                  //structure to store the information of passengers
    char name[50];                //passenger name
    int age;                    
    float height;                // Height in feet discount on height less than 3ft
    char paymentMethod[10];      //cash or card
    char endingStop[20];         //ending stop
    int busNumber;               //assigned based on vacant buses
    int seatNumber;              //assigned based on vacant seats
    int customerID;              //random id to every customer
    float distance;              //per stop  distance is 10 km
    float fare;                  //to be calculated based on total km and speed
    char bookingTime[50];        //to include real booking time
    char discountDetails[200];   // Stores discount details
    double cnic;                 //takes cnic input
} Passenger;

typedef struct {
    double cnic;        //for loyalty customers profile
    char name[50];
    int tripsCompleted;      //no of trips completed
    float totalDistance;     //total distance of trips
    float totalFarePaid;     //total fare paid
} UserProfile;

// Global variables can't be changed during program 
int seats[MAX_BUSES][SEATS_PER_BUS] = {0};    // 0 = vacant, 1 = booked
int passengerCount = 0;

const char *stops[MAX_STOPS] = {"Stop A", "Stop B", "Stop C", "Stop D", "Stop E"};        //const so they can't be changed at any point

// Functions to be used in the program
void displayscreen();                      //to display the icon and name
void displayMenu();                       //to display menu to user
void bookTicket(int stopIndex);           //when user wants to book ticket
void saveToFile(Passenger passenger);     //to save user data in profile
void adminPanel();                        //to view the admin profile
void viewDailyRecords();                  //to display the records of user travelled in a single day
void resetDailyRecords();                 //to reset the daily records for memory cleaning(to be used by admin only)                  //to save the data of loyal customers
void displayStops();   
int findUserByCNIC(double cnic, UserProfile *user);                                   //display total stops to user
int generateCustomerID();                                //to generate a unique id to user
void getNewCustomerDetails(Passenger *customer, int stopIndex);   //to take input of new users information
float applyDiscount(float fare, float height, char paymentMethod[], int numberOfStops, char* discountDetails);   //apply different discounts,i.e;age,height,payment etc
float applyPeakHourCharge(float fare);                 //for calculating peak hour fare i.e;5-8pm
void processPassengersAtStop(int stopIndex);           //to check for passengers in different stops
void saveSeatsToFile();
void loadSeatsFromFile();
int findUserInPassengers(double cnic, Passenger *passenger);
void updateLoyaltyData(double cnic, float fare, float distance);
void viewloyaltyprofiles();
void viewsummary();

//functions for validating inputs

void validate_cnic(double *cnic);
void validatePayment(char *method);

//Now defining fuctions one by one

void displayMenu() {  
    printf("\n\tMENU: \n\n");          //displays the menu and ask user to choose 1 option;
    printf("1. Start Boarding\n");     
    printf("2. Admin Panel\n");
    printf("3. Exit\n");
    printf("\nEnter your choice: ");
}

void displayStops() {             //displays all the available stops
    printf("\nAvailable Stops:\n");
    for (int i = 0; i < MAX_STOPS; i++) {
        printf("%d. %s\n", i + 1, stops[i]);
    }
}

void bookTicket(int stopIndex) {       //stopIndex is the current stop 
    Passenger passenger;              //instance of structure Passenger
 time_t t;
    
    passenger.customerID = generateCustomerID();     // Generates new customer ID
    printf("Your new customer ID is: %d\n", passenger.customerID);

    // Get new customer details
    getNewCustomerDetails(&passenger, stopIndex);

    // Get ending stop
    int endChoice;
do {
    printf("Select the ending stop (cannot be the current stop %d): ", stopIndex + 1);
    if (scanf("%d", &endChoice) != 1) { // Check if the input is a valid integer
        printf("Invalid input. Please enter a valid stop number.\n");
        while (getchar() != '\n'); // Clear input buffer
        continue;
    }

    if (endChoice == stopIndex + 1) { // Check if the user selects the current stop
        printf("Invalid stop selection. The ending stop cannot be the current stop.\n");
    } else if (endChoice < 1 || endChoice > MAX_STOPS) { // Ensure stop is within valid range
        printf("Invalid stop selection. Please choose a stop between 1 and %d.\n", MAX_STOPS);
    } else {
        break; // Valid input
    }
} while (1);


    strcpy(passenger.endingStop, stops[endChoice - 1]);

    // Calculate the number of stops
    // Calculate the number of stops (absolute distance between stops)
int numberOfStops;

// Check if the user is traveling in reverse
if (endChoice < (stopIndex + 1)) {
    // Calculate total stops including reverse route
    numberOfStops = (MAX_STOPS - (stopIndex + 1)) + endChoice; // Forward to Stop 5 and reverse to the ending stop
} else {
    // Normal forward travel
    numberOfStops = abs(endChoice - (stopIndex + 1));
}

// Calculate distance and fare
passenger.distance = numberOfStops * 10.0; // Assuming 10 km per stop
passenger.fare = BASE_FARE + (numberOfStops - 1) * ADDITIONAL_STOP_FARE;

// Apply discounts
passenger.fare = applyDiscount(passenger.fare, passenger.height, passenger.paymentMethod, numberOfStops, passenger.discountDetails);

// Apply peak hour charges
passenger.fare = applyPeakHourCharge(passenger.fare);

    // Assign seat and bus
    int seat_assigned=0;
    
    for (int i = 0; i < MAX_BUSES; i++) {
        for (int j = 0; j < SEATS_PER_BUS; j++) {
            if (seats[i][j] == 0) {           // Vacant seat
                passenger.busNumber = i + 1;
                passenger.seatNumber = j + 1;
                seats[i][j] = 1;   
				seat_assigned=1; 
				break;                  // Mark seat as booked
            }
        }
        if(seat_assigned)
        break;
    }
    if(!seat_assigned){
    printf("All seats are full!\n");
    return;
	}
	
	 saveSeatsToFile();

    time(&t);  //to record the real time
    strcpy(passenger.bookingTime, ctime(&t)); 
    passenger.bookingTime[strcspn(passenger.bookingTime, "\n")] = '\0'; // Remove newline

    // Save the passenger record  to file
    saveToFile(passenger);
    updateLoyaltyData(passenger.cnic, passenger.fare, passenger.distance);

    
    // Display booking details and discount information
    printf("\n\t...Booking Successful!......\n");
    printf("Name: %s\nBus Number: %d\nSeat Number: %d\nCustomer ID: %d\n", 
           passenger.name, passenger.busNumber, passenger.seatNumber, passenger.customerID);
    printf("Booking Time: %s\nEstimated Fare: $%.2f\n", passenger.bookingTime, passenger.fare);
    printf("Discounts Applied: %s\n", passenger.discountDetails);
}

void saveSeatsToFile() {
    FILE *file = fopen("seats.txt", "w");
    if (!file) {
        printf("Error saving seats state to file.\n");
        return;
    }

    // Write the seats array to the file
    for (int i = 0; i < MAX_BUSES; i++) {
        for (int j = 0; j < SEATS_PER_BUS; j++) {
            fprintf(file, "%d ", seats[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void load_seatsfromfile() {
    FILE *file = fopen("seats.txt", "r");
    if (!file) {
        printf("No previous seats state found. Starting fresh.\n");
        return;
    }

    // Read the seats array from the file
    for (int i = 0; i < MAX_BUSES; i++) {
        for (int j = 0; j < SEATS_PER_BUS; j++) {
            fscanf(file, "%d", &seats[i][j]);
        }
    }

    fclose(file);
}

void saveToFile(Passenger passenger) {
    FILE *file = fopen("passenger_records.txt", "a");
    if (!file) {
        printf("Error saving to file. Ensure the file is writable.\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fprintf(file, "Name           | Age | Height | Payment | End Stop  | Bus No. | Seat No. | CNIC          | Fare  | Booking Time\n");
        fprintf(file, "---------------------------------------------------------------------------------------------\n");
    }

    fprintf(file, "%-15s | %-3d | %-6.2f | %-7s | %-9s | %-7d | %-8d | %-13.0lf | $%-5.2f | %s\n",
            passenger.name, passenger.age, passenger.height, passenger.paymentMethod,
            passenger.endingStop, passenger.busNumber, passenger.seatNumber,
            passenger.cnic, passenger.fare, passenger.bookingTime);

    fclose(file);
}

int generateCustomerID() {
    // Generate a random customer ID between 1000 and 9999
    return (rand() % (CUSTOMER_ID_MAX - CUSTOMER_ID_MIN + 1)) + CUSTOMER_ID_MIN;   //expression for random customer id
}

void getNewCustomerDetails(Passenger *customer, int stopIndex) {
    

    printf("\nEnter your CNIC number (without dashes): ");
    scanf("%lf", &customer->cnic);
    validate_cnic(&customer->cnic);

        printf("Enter your name: ");
        scanf(" %[^\n]", customer->name);  // Read the name with spaces
        do {
            printf("Enter your age: ");
            scanf("%d", &customer->age);
            if (customer->age < 0) {
                printf("\nInvalid age entered! Try again\n");
            }
        } while (customer->age < 0);

        do {
            printf("Enter your height (in feet): ");
            scanf("%f", &customer->height);
            if (customer->height <= 0) {
                printf("\nInvalid height! Try again\n");
            }
        } while (customer->height <= 0);

        printf("Enter your payment method (cash/card): ");
        scanf("%s", customer->paymentMethod);
        validatePayment(customer->paymentMethod);
    }

float applyDiscount(float fare, float height, char paymentMethod[], int numberOfStops, char* discountDetails){            //used pointer for discountDetails b\c it will be updated and used in main
    int discountApplied = 0;

    // Applying a 3% discount if the payment method is a card
    if (strcmp(paymentMethod, "card") == 0) {
        fare = fare * 0.97;          // Apply 3% discount
        strcat(discountDetails, "Card payment (3%)");   
        discountApplied = 1;
    }

    // Applying a 3% discount if traveling more than 2 stops
    if (numberOfStops > 2) {
        fare = fare * 0.97; // Apply 3% discount
        if (discountApplied) strcat(discountDetails, ", ");
        strcat(discountDetails, "More than 2 stops (3%)");
        discountApplied = 1;
    }

    // Apply a 3% discount if the height is less than 3 feet
    if (height < DISCOUNT_HEIGHT) {
        fare = fare * 0.97;    // Apply 3% discount
        if (discountApplied) strcat(discountDetails, ", ");
        strcat(discountDetails, "Height less than 3 feet (3%)");
    }

    return fare;    //returns the updated fare with discount if added
}

float applyPeakHourCharge(float fare) {
    time_t t;
    struct tm *tm_info;    //to get the real time
    time(&t);
    tm_info = localtime(&t);

    // Check if the time is between 5 PM and 8 PM
    if (tm_info->tm_hour >= PEAK_HOUR_START && tm_info->tm_hour < PEAK_HOUR_END) {
        fare = fare * 1.05;          // Applying a 5% peak hour surcharge
        printf("Peak hour charge applied (5%%)\n");
    }

    return fare;
}

void processPassengersAtStop(int stopIndex) {   // to check for passengers at each stop
    char choice;
    
    printf("\n\t......Bus is currently waiting at stop %s........",stops[stopIndex]);
     printf("\n\t..........Now boarding passengers at %s..........\n", stops[stopIndex]);
     
    do {
        printf("\n\tDo you want to book a ticket for this stop? [y for (yes)/n for (no)]: "); //y for yes , n for no
        scanf(" %c", &choice);

        if (choice == 'y' || choice == 'Y') {   // if choice = y or Y then again display the book ticket function
            bookTicket(stopIndex);
        } else if (choice=='n' || choice=='N') {
            printf("\nFinished boarding at %s.\n.........Bus is now moving to the next stop............\n", stops[stopIndex]); // if no more customer want to go from stop1 bus starts moving to stop 2
        }
        else {
        	printf("\nInvalid Choice! Enter choice again(y/n): \n");
		}
    } while (choice != 'n' && choice != 'N');
}

void adminPanel() {
    char password[20];
    printf("Enter admin password: ");
    scanf("%s", password);

    if (strcmp(password, "admin123") == 0) {   //password for admin = admin123
        int choice;
        printf("\nAdmin Panel\n");
        printf("1. View Daily Records\n");
        printf("2. Reset Records\n");
        printf("3. View Loyalty Profiles\n");
        printf("4.View Summary\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                viewDailyRecords();           //views the passenger records file
                break;
            case 2:
                resetDailyRecords();          //resets the existing records
                break;
            case 3:
            	viewloyaltyprofiles();
            	break;
            case 4:
            	viewsummary();
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
    if (!file) {               //if no record is present 
        printf("No records found.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {   //gets the data from file 
        printf("%s", line);
    }

    fclose(file);   //close the file after execution
}

void resetDailyRecords() {
    FILE *file = fopen("passenger_records.txt", "w");  //to reset the passenger records
    if (!file) {
        printf("Error resetting records.\n");     //opening the file in "w" mode b\c it overwrites the previous contents
        return;                                  
    }
    fclose(file);
    printf("Daily records reset successfully.\n");
}

void viewloyaltyprofiles() {                     //to display loyal customers data
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

void validatePayment(char *method) {
    while (strcmp(method, "cash") != 0 && strcmp(method, "card") != 0) {
        printf("\nInvalid payment method: Enter cash or card: ");
        scanf("%s", method);
    }
}
void validate_cnic(double *cnic) {
    char cnicStr[20];  // To store CNIC as a string for validation
    int isValid = 0;

    while (!isValid) {
        sprintf(cnicStr, "%.0lf", *cnic);  // Convert CNIC to string for validation
        if (strlen(cnicStr) == 13 && strspn(cnicStr, "0123456789") == 13) {
            isValid = 1;  // CNIC is valid
        } else {
            printf("\nInvalid CNIC number. Must be 13 digits. Try again: ");
            scanf("%lf", cnic);  // Re-prompt user for CNIC input
        }
    }
}

int findUserInPassengers(double cnic, Passenger *passenger) {
    FILE *file = fopen("passenger_records.txt", "r");
    if (!file) {
        return 0; // No record found
    }

    char line[256];
     for (int i = 0; i < 4; i++) {
        fgets(line, sizeof(line), file);  // Read and ignore the first three lines(header)
    }

    while (fscanf(file, "%49s %*d %*f %*s %*s %*d %*d %lf %f %s", 
                  passenger->name, &passenger->cnic, &passenger->fare, passenger->bookingTime) != EOF) {
        if (passenger->cnic == cnic) {
            fclose(file);
            return 1; // User found
        }
    }

    fclose(file);
    return 0; // User not found
}

void updateLoyaltyData(double cnic, float fare, float distance) {
    FILE *file = fopen("loyalty_data.txt", "r+");
    if (!file) {
        // Create loyalty file if it doesn't exist
        file = fopen("loyalty_data.txt", "w");
        fprintf(file, "CNIC           | Trips | Total Fare | Total Distance\n");
        fprintf(file, "---------------------------------------------------\n");
        fclose(file);
        file = fopen("loyalty_data.txt", "r+");
    }

    char line[256];
    int userFound = 0;

    // Temporary file for updating data
    FILE *tempFile = fopen("loyalty_data_temp.txt", "w");
    if (!tempFile) {
        printf("Error creating temporary loyalty file.\n");
        fclose(file);
        return;
    }

    // Process existing data
    while (fgets(line, sizeof(line), file)) {
        double fileCnic;
        int trips;
        float totalFare, totalDistance;

        if (sscanf(line, "%lf %d %f %f", &fileCnic, &trips, &totalFare, &totalDistance) == 4) {
            if (fileCnic == cnic) {
                userFound = 1;
                trips++;
                totalFare += fare;
                totalDistance += distance;
                fprintf(tempFile, "%-17.0lf %-6d %-12.2f %-14.2f\n", cnic, trips, totalFare, totalDistance);
            } else {
                fprintf(tempFile, "%s", line); // Copy other data as is
            }
        } else {
            fprintf(tempFile, "%s", line); // Copy header or invalid lines
        }
    }

    // Add new user to loyalty file if not found
    if (!userFound) {
        fprintf(tempFile, "%-17.0lf %-6d %-12.2f %-14.2f\n", cnic, 1, fare, distance);
    }

    fclose(file);
    fclose(tempFile);

    // Replace old file with updated file
    if (remove("loyalty_data.txt") != 0) {
        printf("Error removing old loyalty file.\n");
    }
    if (rename("loyalty_data_temp.txt", "loyalty_data.txt") != 0) {
        printf("Error renaming temporary loyalty file.\n");
    }

    printf("Loyalty data updated successfully.\n");
}

void viewsummary() {
    FILE *file = fopen("passenger_records.txt", "r");
    if (!file) {
        printf("No records found.\n");
        return;
    }

    char line[256];
    int totalUsers = 0;
    float totalFare = 0.0;

    for (int i = 0; i < 4; i++) {
        fgets(line, sizeof(line), file);  // Read and ignore the first three lines(header)
    }

    float fare = 0.0;
    while (fgets(line, sizeof(line), file)) {
        totalUsers++;       // Count each user

        // converting the lines into string to deal with the $icon
        char *farePtr = strstr(line, "$");    // Find the dollar sign
        if (farePtr) {
            sscanf(farePtr + 1, "%f", &fare);   // Read the number after the dollar sign
            totalFare += fare;                 // Add the fare to the total
        }
    }

    fclose(file);

    // Display the summary
    printf("\nSummary:\n");
    printf("Total number of users: %d\n", totalUsers);
    printf("Total fare collected: $%.2f\n", totalFare);
}



void displayscreen(){              //function to display welcome screen consisting of system name and a bus icon
	printf("\n");
	printf("                                      \n");
	printf("  WELCOME TO RIDEBUDDY\n");
	printf("\n   public transport management system\n");
	printf("\t\t\t\t      ____________________              \n");
	printf("\t\t\t\t     /   ___        ___   \\            \n");   //bus icon
	printf("\t\t\t\t    |  | O |      | O |   |            \n");
	printf("\t\t\t\t    |   |_O_|      |_O_| _|           \n");
	printf("\t\t\t\t     \\____________________|          \n");                   
	printf("\t\t\t\t           0           0               \n");
	
}

int main() {
    srand(time(NULL));               ////to ensure each customer gets a unique and different customer id
	load_seatsfromfile();         
    int choice;
     displayscreen();            //displays the welcome screen with name and icon
    do {
        displayMenu();           //displays the menu consisting of booking and admin choice options
        scanf("%d", &choice);  

        switch (choice) {
        	   
            case 1:                                      //if choice is "Boarding a ticket" 
                for (int i = 0; i < MAX_STOPS; i++) {   //to iterate through each stop i.e 5
                    processPassengersAtStop(i);         //to check for customer in each stop
                }  
                break;
            case 2:                //if choice is "Admin panel"
                adminPanel();     //displaying the admin panel with functions 
                break;
            case 3:
                printf("Exiting...\n");    //if choice is "3" exit the system
                break;
            default:
                printf("Invalid choice.\n");   //if choice is not between (1-3)
        }
    } while (choice != 3);          //the system runs until user wants to exit the system

    return 0;
}     