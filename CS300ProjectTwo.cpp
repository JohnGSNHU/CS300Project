/*
Arthor: John Grzeskowiak
Date: 2023-10-01
Title: Hash Table Implementation for Course Advising Program
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;
// Structure to represent a course with its number, title, and prerequisites
struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};
// Function to trim whitespace from both ends of a string
string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");

    if (start == string::npos) {
        return "";
    }

    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}
// Function to split a line into fields based on commas, handling empty fields
vector<string> splitLine(const string& line) {
    vector<string> fields;
    string field;
    stringstream stream(line);

    while (getline(stream, field, ',')) {
        fields.push_back(trim(field));
    }
	// Handle the case where the last field is empty/ends with comma (e.g., "CS101,Intro to CS,")
    if (!line.empty() && line.back() == ',') {
        fields.push_back("");
    }

    return fields;
}
// Hash table implementation for storing courses
class CourseHashTable {
private:
    static const size_t TABLE_SIZE = 179;
    vector<vector<Course>> table;
    size_t courseCount = 0;
	// Converts a course number into an index for the hash table using a hash function
    size_t hashFunction(const string& courseNumber) const {
        return hash<string>{}(courseNumber) % table.size();
    }

public:
    CourseHashTable() : table(TABLE_SIZE) {}

	// Inserts a course into the hash table, returning false if the course number already exists
    bool insert(const Course& course) {
        size_t index = hashFunction(course.courseNumber);

        for (const Course& existing : table[index]) {
            if (existing.courseNumber == course.courseNumber) {
                return false;
            }
        }

        table[index].push_back(course);
        courseCount++;
        return true;
    }
	// Finds a course in the hash table by its course number, returning a pointer to the course or nullptr if not found
    const Course* find(const string& courseNumber) const {
        size_t index = hashFunction(courseNumber);

        for (const Course& course : table[index]) {
            if (course.courseNumber == courseNumber) {
                return &course;
            }
        }

        return nullptr;
    }
	// Retrieves all courses from the hash table as a vector
    vector<Course> getAllCourses() const {
        vector<Course> courses;
        courses.reserve(courseCount);

        for (const auto& bucket : table) {
            for (const Course& course : bucket) {
                courses.push_back(course);
            }
        }

        return courses;
    }

    bool isEmpty() const {
        return courseCount == 0;
    }

    size_t size() const {
        return courseCount;
    }
	// Swaps the contents of this hash table with another hash table
    void swap(CourseHashTable& other) {
        table.swap(other.table);
        std::swap(courseCount, other.courseCount);
    }
};
// Function to load courses from a CSV file into the hash table, returning false if any errors occur
bool loadCourses(CourseHashTable& courses, const string& fileName) {
    ifstream inputFile(fileName);

    if (!inputFile) {
        cout << "Error: Unable to open file.\n";
        return false;
    }
	// Create a new hash table to store the courses temporarily
    CourseHashTable newCourses;
    string line;
    int lineNumber = 0;

    while (getline(inputFile, line)) {
        lineNumber++;
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        vector<string> fields = splitLine(line);
		// Validate that the line has at least two fields (course number and title) and that they are not empty
        if (fields.size() < 2 ||
            fields[0].empty() || fields[1].empty()) {
            cout << "Error: Invalid file format on line "
                << lineNumber << ".\n";
            return false;
        }

        Course course;
        course.courseNumber = fields[0];
        course.title = fields[1];

        for (size_t i = 2; i < fields.size(); i++) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(fields[i]);
            }
        }

        if (!newCourses.insert(course)) {
            cout << "Error: Duplicate course number "
                << course.courseNumber << ".\n";
            return false;
        }
    }

    if (newCourses.isEmpty()) {
        cout << "Error: No course data found in file.\n";
        return false;
    }
	// Validate that all prerequisites exist in the newCourses hash table
    for (const Course& course : newCourses.getAllCourses()) {
        for (const string& prerequisite : course.prerequisites) {
            if (newCourses.find(prerequisite) == nullptr) {
                cout << "Error: Prerequisite " << prerequisite
                    << " for course " << course.courseNumber
                    << " does not exist.\n";
                return false;
            }
        }
    }
    //
    courses.swap(newCourses);

    cout << "Course data loaded successfully.\n";
    cout << "Number of courses loaded: "
        << courses.size() << '\n';

    return true;
}
// Function to print detailed information about a specific course, including its prerequisites
void printCourseInformation(const CourseHashTable& courses) {
    string courseNumber;

    cout << "Enter course number: ";
    getline(cin, courseNumber);
    courseNumber = trim(courseNumber);

    const Course* course = courses.find(courseNumber);

    if (course == nullptr) {
        cout << "Error: Course not found.\n";
        return;
    }

    cout << "\nCourse Number: " << course->courseNumber << '\n';
    cout << "Course Title: " << course->title << '\n';

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }

    cout << "Prerequisites:\n";

    for (const string& number : course->prerequisites) {
        const Course* prerequisite = courses.find(number);

        if (prerequisite != nullptr) {
            cout << "  " << prerequisite->courseNumber
                << " - " << prerequisite->title << '\n';
        }
    }
}
//
void printCourseList(const CourseHashTable& courses) {
    vector<Course> sortedCourses = courses.getAllCourses();

    sort(sortedCourses.begin(), sortedCourses.end(),
        [](const Course& first, const Course& second) {
            return first.courseNumber < second.courseNumber;
        });

    cout << "\nCourse List (Alphanumeric Order)\n";

    for (const Course& course : sortedCourses) {
        cout << course.courseNumber << " - "
            << course.title << '\n';
    }
}

int main() {
    CourseHashTable courses;
    string choice;
    const string fileName = "CS 300 ABCU_Advising_Program_Input.csv";
	//main menu loop and options
    while (true) {
        cout << "\nABCU Course Advising Program\n";
        cout << "1. Load Course Data\n";
        cout << "2. Print Course List\n";
        cout << "3. Print Course Information\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";

        getline(cin, choice);
        choice = trim(choice);

        if (choice == "1") {
            loadCourses(courses, fileName);
        }

        else if (choice == "2") {
            if (courses.isEmpty()) {
                cout << "Error: Please load course data first.\n";
            }
            else {
                printCourseList(courses);
            }
        }

        else if (choice == "3") {
            if (courses.isEmpty()) {
                cout << "Error: Please load course data first.\n";
            }
            else {
                printCourseInformation(courses);
            }
        }

        else if (choice == "9") {
            cout << "Thank you for using the ABCU Course Advising Program.\n";
            break;
        }
        else {
            cout << "Error: Invalid menu choice. Please try again.\n";
        }
    }

    return 0;
}