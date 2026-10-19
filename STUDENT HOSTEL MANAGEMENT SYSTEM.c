
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STUDENT_FILE   "students.dat"
#define ROOM_FILE      "rooms.dat"
#define TOTAL_ROOMS    20
#define ROOM_CAPACITY  4
#define NAME_LEN       50
#define COURSE_LEN     30
#define CONTACT_LEN    15

/* ---------------- Data Structures ---------------- */

typedef struct {
    int  id;
    char name[NAME_LEN];
    char course[COURSE_LEN];
    char contact[CONTACT_LEN];
    int  roomNo;
    float feesPaid;
    float feesDue;
    int  isActive;   /* 1 = active record, 0 = deleted */
} Student;

typedef struct {
    int roomNo;
    int capacity;
    int occupied;
} Room;

/* ---------------- Function Prototypes ---------------- */

void initializeRooms(void);
void addStudent(void);
void viewAllStudents(void);
void searchStudent(void);
void updateStudent(void);
void deleteStudent(void);
void viewRoomStatus(void);
void payFees(void);
int  getNextStudentId(void);
int  allocateRoom(void);
void deallocateRoom(int roomNo);
void pause_screen(void);
void printHeader(const char *title);
void searchstudentbyname(void);


/* ================================================================
   MAIN MENU
   ================================================================ */
int main(void) {
    initializeRooms();
    int choice;

    do {
        printHeader("STUDENT HOSTEL MANAGEMENT SYSTEM");
        printf(" 1. Add New Student\n");
        printf(" 2. View All Students\n");
        printf(" 3. Search Student by ID\n");
        printf(" 4. Update Student Record\n");
        printf(" 5. Delete Student Record\n");
        printf(" 6. View Room Allocation Status\n");
        printf(" 8. Search Student by name\n");
        printf(" 7. Record Fee Payment\n");
        printf(" 0. Exit\n");
        printf("----------------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            /* clear invalid input */
            while (getchar() != '\n');
            choice = -1;
        }

        switch (choice) {
            case 1: addStudent();        break;
            case 2: viewAllStudents();   break;
            case 3: searchStudent();     break;
            case 4: updateStudent();     break;
            case 5: deleteStudent();     break;
            case 6: viewRoomStatus();    break;
            case 7: payFees();           break;
            case 8: searchstudentbyname(); break;
            case 0: printf("\nExiting... All data has been saved.\n"); break;
            default: printf("\nInvalid choice. Please try again.\n"); pause_screen();
        }
    } while (choice != 0);

    return 0;
}

/* ================================================================
   UTILITY FUNCTIONS
   ================================================================ */

void printHeader(const char *title) {
    printf("\n================================================\n");
    printf("   %s\n", title);
    printf("================================================\n");
}

void pause_screen(void) {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n');
    getchar();
}


void initializeRooms(void) {
    FILE *fp = fopen(ROOM_FILE, "rb");
    if (fp != NULL) {
        fclose(fp);
        return;
    }

    fp = fopen(ROOM_FILE, "wb");
    if (fp == NULL) {
        printf("Error: could not create room file.\n");
        exit(1);
    }

    Room r;
    for (int i = 1; i <= TOTAL_ROOMS; i++) {
        r.roomNo   = i;
        r.capacity = ROOM_CAPACITY;
        r.occupied = 0;
        fwrite(&r, sizeof(Room), 1, fp);
    }
    fclose(fp);
}


int getNextStudentId(void) {
    FILE *fp = fopen(STUDENT_FILE, "rb");
    int maxId = 1000; /* IDs start at 1001 */
    if (fp == NULL) return maxId + 1;

    Student s;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.id > maxId) maxId = s.id;
    }
    fclose(fp);
    return maxId + 1;
}


int allocateRoom(void) {
    FILE *fp = fopen(ROOM_FILE, "rb+");
    if (fp == NULL) return -1;

    Room r;
    long pos = 0;
    int chosenRoom = -1;

    while (fread(&r, sizeof(Room), 1, fp) == 1) {
        if (r.occupied < r.capacity) {
            r.occupied++;
            chosenRoom = r.roomNo;
            fseek(fp, pos, SEEK_SET);
            fwrite(&r, sizeof(Room), 1, fp);
            break;
        }
        pos += sizeof(Room);
    }
    fclose(fp);
    return chosenRoom;
}

/* Free up one bed in the given room number */
void deallocateRoom(int roomNo) {
    FILE *fp = fopen(ROOM_FILE, "rb+");
    if (fp == NULL) return;

    Room r;
    while (fread(&r, sizeof(Room), 1, fp) == 1) {
        if (r.roomNo == roomNo) {
            if (r.occupied > 0) r.occupied--;
            fseek(fp, -(long)sizeof(Room), SEEK_CUR);
            fwrite(&r, sizeof(Room), 1, fp);
            break;
        }
    }
    fclose(fp);
}

/* ================================================================
   CORE FEATURES
   ================================================================ */

void addStudent(void) {
    Student s;
    printHeader("ADD NEW STUDENT");

    int room = allocateRoom();
    if (room == -1) {
        printf("Sorry, the hostel is completely full. Cannot admit new student.\n");
        pause_screen();
        return;
    }

    s.id = getNextStudentId();
    getchar(); /* consume leftover newline from previous scanf */

    printf("Student ID (auto-generated): %d\n", s.id);

    printf("Enter Name: ");
    fgets(s.name, NAME_LEN, stdin);
    s.name[strcspn(s.name, "\n")] = '\0';

    printf("Enter Course: ");
    fgets(s.course, COURSE_LEN, stdin);
    s.course[strcspn(s.course, "\n")] = '\0';

    printf("Enter Contact Number: ");
    fgets(s.contact, CONTACT_LEN, stdin);
    s.contact[strcspn(s.contact, "\n")] = '\0';

    s.roomNo    = room;
    s.feesPaid  = 0.0f;
    s.feesDue   = 0.0f;
    s.isActive  = 1;

    printf("Enter Total Hostel Fee Due: ");
    scanf("%f", &s.feesDue);

    FILE *fp = fopen(STUDENT_FILE, "ab");
    if (fp == NULL) {
        printf("Error opening student file.\n");
        pause_screen();
        return;
    }
    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);

    printf("\nStudent added successfully! Allocated Room No: %d\n", room);
    pause_screen();
}

void viewAllStudents(void) {
    printf("ALL STUDENT RECORDS\n");
    FILE *fp = fopen(STUDENT_FILE, "rb");
    if (fp == NULL) {
        printf("No records found.\n");
        pause_screen();
        return;
    }

    Student s;
    int count = 0;
    printf("%-6s %-20s %-15s %-12s %-6s %-10s %-10s\n",
           "ID", "Name", "Course", "Contact", "Room", "Paid", "Due");
    printf("------------------------------------------------------------------------\n");

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.isActive) {
            printf("%-6d %-20s %-15s %-12s %-6d %-10.2f %-10.2f\n",
                   s.id, s.name, s.course, s.contact, s.roomNo, s.feesPaid, s.feesDue);
            count++;
        }
    }
    fclose(fp);

    if (count == 0) printf("No active student records found.\n");
    pause_screen();
}

void searchStudent(void) {
    printHeader("SEARCH STUDENT BY ID");
    int id;
    printf("Enter Student ID: ");
    scanf("%d", &id);

    FILE *fp = fopen(STUDENT_FILE, "rb");
    if (fp == NULL) {
        printf("No records found.\n");
        pause_screen();
        return;
    }

    Student s;
    int found = 0;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.id == id && s.isActive) {
            found = 1;
            printf("\nID          : %d\n", s.id);
            printf("Name        : %s\n", s.name);
            printf("Course      : %s\n", s.course);
            printf("Contact     : %s\n", s.contact);
            printf("Room No     : %d\n", s.roomNo);
            printf("Fees Paid   : %.2f\n", s.feesPaid);
            printf("Fees Due    : %.2f\n", s.feesDue);
            break;
        }
    }
    fclose(fp);

    if (!found) printf("\nNo active student found with ID %d.\n", id);
    pause_screen();
}

    void searchstudentbyname(void) {
    printHeader("SEARCH STUDENT BY NAME");

    char name[NAME_LEN];
    char searchName[NAME_LEN];

    printf("Enter Student Name: ");
    getchar();
    fgets(name, sizeof(name), stdin);


    name[strcspn(name, "\n")] = '\0';


    strcpy(searchName, name);

    for (int i = 0; searchName[i] != '\0'; i++) {
        searchName[i] = tolower(searchName[i]);
    }

    FILE *fp = fopen(STUDENT_FILE, "rb");

    if (fp == NULL) {
        printf("No records found.\n");
        pause_screen();
        return;
    }

    Student s;
    int found = 0;

    while (fread(&s, sizeof(Student), 1, fp) == 1) {

        char studentName[NAME_LEN];


        strcpy(studentName, s.name);


        for (int i = 0; studentName[i] != '\0'; i++) {
            studentName[i] = tolower(studentName[i]);
        }


        if (strcmp(studentName, searchName) == 0 && s.isActive) {
            found = 1;

            printf("\n----------------------------------------\n");
            printf("ID          : %d\n", s.id);
            printf("Name        : %s\n", s.name);
            printf("Course      : %s\n", s.course);
            printf("Contact     : %s\n", s.contact);
            printf("Room No     : %d\n", s.roomNo);
            printf("Fees Paid   : %.2f\n", s.feesPaid);
            printf("Fees Due    : %.2f\n", s.feesDue);
            printf("----------------------------------------\n");
        }
    }

    fclose(fp);

    if (found == 0) {
        printf("\nNo active student found with name \"%s\".\n", name);
    }

    pause_screen();
}

void updateStudent(void) {
    printHeader("UPDATE STUDENT RECORD");
    int id;
    printf("Enter Student ID to update: ");
    scanf("%d", &id);
    getchar();

    FILE *fp = fopen(STUDENT_FILE, "rb+");
    if (fp == NULL) {
        printf("No records found.\n");
        pause_screen();
        return;
    }

    Student s;
    int found = 0;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.id == id && s.isActive) {
            found = 1;
            printf("Leave field blank and press Enter to keep current value.\n");

            printf("Name [%s]: ", s.name);
            char buf[NAME_LEN];
            fgets(buf, NAME_LEN, stdin);
            buf[strcspn(buf, "\n")] = '\0';
            if (strlen(buf) > 0) strcpy(s.name, buf);

            printf("Course [%s]: ", s.course);
            fgets(buf, COURSE_LEN, stdin);
            buf[strcspn(buf, "\n")] = '\0';
            if (strlen(buf) > 0) strcpy(s.course, buf);

            printf("Contact [%s]: ", s.contact);
            fgets(buf, CONTACT_LEN, stdin);
            buf[strcspn(buf, "\n")] = '\0';
            if (strlen(buf) > 0) strcpy(s.contact, buf);

            fseek(fp, -(long)sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            printf("\nRecord updated successfully.\n");
            break;
        }
    }
    fclose(fp);

    if (!found) printf("\nNo active student found with ID %d.\n", id);
    pause_screen();
}

void deleteStudent(void) {
    printHeader("DELETE STUDENT RECORD");
    int id;
    printf("Enter Student ID to delete: ");
    scanf("%d", &id);

    FILE *fp = fopen(STUDENT_FILE, "rb+");
    if (fp == NULL) {
        printf("No records found.\n");
        pause_screen();
        return;
    }

    Student s;
    int found = 0;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.id == id && s.isActive) {
            found = 1;
            s.isActive = 0; /* soft delete keeps history but frees the room */
            fseek(fp, -(long)sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);
            deallocateRoom(s.roomNo);
            printf("\nStudent record deleted and Room %d vacated.\n", s.roomNo);
            break;
        }
    }
    fclose(fp);

    if (!found) printf("\nNo active student found with ID %d.\n", id);
    pause_screen();
}

void viewRoomStatus(void) {
    printHeader("ROOM ALLOCATION STATUS");
    FILE *fp = fopen(ROOM_FILE, "rb");
    if (fp == NULL) {
        printf("Room data not found.\n");
        pause_screen();
        return;
    }

    Room r;
    printf("%-10s %-10s %-10s %-10s\n", "Room No", "Capacity", "Occupied", "Free Beds");
    printf("----------------------------------------------\n");
    while (fread(&r, sizeof(Room), 1, fp) == 1) {
        printf("%-10d %-10d %-10d %-10d\n",
               r.roomNo, r.capacity, r.occupied, r.capacity - r.occupied);
    }
    fclose(fp);
    pause_screen();
}

void payFees(void) {
    printHeader("RECORD FEE PAYMENT");
    int id;
    float amount;
    printf("Enter Student ID: ");
    scanf("%d", &id);

    FILE *fp = fopen(STUDENT_FILE, "rb+");
    if (fp == NULL) {
        printf("No records found.\n");
        pause_screen();
        return;
    }

    Student s;
    int found = 0;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.id == id && s.isActive) {
            found = 1;
            printf("Current Fees Due: %.2f\n", s.feesDue);
            printf("Enter payment amount: ");
            scanf("%f", &amount);

            if (amount > s.feesDue) {
                printf("Amount exceeds fees due. Payment capped at due amount.\n");
                amount = s.feesDue;
            }

            s.feesPaid += amount;
            s.feesDue  -= amount;

            fseek(fp, -(long)sizeof(Student), SEEK_CUR);
            fwrite(&s, sizeof(Student), 1, fp);

            printf("\nPayment recorded. New balance due: %.2f\n", s.feesDue);
            break;
        }
    }
    fclose(fp);

    if (!found) printf("\nNo active student found with ID %d.\n", id);
    pause_screen();
}
