#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to store user data
typedef struct {
    char gender[10];
    char goal[20];
} User;

// Function prototypes
void displayDietPlan(User *user);
void displayWorkoutPlan(User *user);
void calorieTracker();
void calculateNetCalories();

int main() {
    User *user = (User *)malloc(sizeof(User)); // Dynamic memory allocation for user

    printf("Enter your gender (Male/Female): ");
    scanf("%s", user->gender);

    printf("Enter your goal (fatloss/muscle gain/both): ");
    scanf(" %[^\n]", user->goal);

    int choice;
    do {
        printf("\nChoose an option:\n");
        printf("1) Diet Plan\n");
        printf("2) Gym Workout\n");
        printf("3) Calorie Tracker\n");
        printf("4) Net Calories\n");
        printf("5) Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayDietPlan(user);
                break;
            case 2:
                displayWorkoutPlan(user);
                break;
            case 3:
                calorieTracker();
                break;
            case 4:
                calculateNetCalories();
                break;
            case 5:
                printf("Exiting the program. Stay healthy!\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 5);

    free(user); // Freeing allocated memory
    return 0;
}

// Function to display diet plan
void displayDietPlan(User *user) {
    char filename[50];
    sprintf(filename, "diet_%s%s.txt", user->goal, user->gender);
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error: Diet plan file not found for %s and %s.\n", user->goal, user->gender);
        return;
    }

    printf("\n7-Day Diet Plan:\n");
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

// Function to display workout plan
void displayWorkoutPlan(User *user) {
    char filename[50];
    sprintf(filename, "workout_%s%s.txt", user->goal, user->gender);
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error: Workout plan file not found for %s and %s.\n", user->goal, user->gender);
        return;
    }

    printf("\n7-Day Gym Workout Plan:\n");
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

// Function for calorie tracker
void calorieTracker() {
    int caloriesConsumed, caloriesBurned;

    printf("\nEnter total calories consumed today: ");
    scanf("%d", &caloriesConsumed);

    printf("Enter total calories burned today: ");
    scanf("%d", &caloriesBurned);

    FILE *file = fopen("calorie_log.txt", "a");
    if (file == NULL) {
        printf("Error: Unable to open calorie log file.\n");
        return;
    }

    fprintf(file, "%d %d\n", caloriesConsumed, caloriesBurned);
    fclose(file);

    printf("Calorie data saved successfully!\n");
}

// Function to calculate net calories
void calculateNetCalories() {
    FILE *file = fopen("calorie_log.txt", "r");
    if (file == NULL) {
        printf("Error: Unable to open calorie log file. Make sure you have logged some data first.\n");
        return;
    }

    int totalConsumed = 0, totalBurned = 0;
    int consumed, burned;

    while (fscanf(file, "%d %d", &consumed, &burned) != EOF) {
        totalConsumed += consumed;
        totalBurned += burned;
    }
    fclose(file);

    int netCalories = totalConsumed - totalBurned;

    printf("\nNet Calorie Summary:\n");
    printf("Total Calories Consumed: %d\n", totalConsumed);
    printf("Total Calories Burned: %d\n", totalBurned);
    printf("Net Calories: %d\n", netCalories);
}
