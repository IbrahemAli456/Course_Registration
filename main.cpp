// CourseRegistrationSystem.cpp


#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <algorithm>


using namespace std;

struct Course {
    string code, title, syllabus;
    int credits;
    string instructor;
    vector<string> prerequisites;
};

struct GradeEntry {
    string courseCode;
    string grade;
    int semester;
};

struct Student {
    string id, name;
    vector<string> registeredCourses;
    vector<GradeEntry> grades;
};

struct Admin {
    string username, password;
};

unordered_map<string, Course> courseCatalog;
unordered_map<string, Student> studentsDB;
unordered_map<string, Admin> adminsDB;

// ---------------------- Utility Functions ----------------------
static void loadCourses() {
    ifstream file("courses.txt");
    string line;
    while (getline(file, line)) {
        Course course;
        stringstream ss(line);
        getline(ss, course.code, ',');
        getline(ss, course.title, ',');
        getline(ss, course.syllabus, ',');
        ss >> course.credits;
        ss.ignore();
        getline(ss, course.instructor, ',');
        string preq;
        while (getline(ss, preq, ',')) course.prerequisites.push_back(preq);
        courseCatalog[course.code] = course;
    }
    file.close();
}

static void saveCourses() {
    ofstream file("courses.txt");
    for (auto it = courseCatalog.begin(); it != courseCatalog.end(); ++it) {
        const string& code = it->first;
        const Course& course = it->second;
        file << course.code << "," << course.title << "," << course.syllabus << "," << course.credits << "," << course.instructor;
        for (size_t i = 0; i < course.prerequisites.size(); ++i) file << "," << course.prerequisites[i];
        file << endl;
    }
    file.close();
}

static void loadStudents() {
    ifstream file("students.txt");
    string line;
    while (getline(file, line)) {
        Student student;
        stringstream ss(line);
        getline(ss, student.id, ',');
        getline(ss, student.name, ',');

        string reg;
        while (getline(ss, reg, ',')) {
            student.registeredCourses.push_back(reg);
        }

        

        studentsDB[student.id] = student;
    }
    file.close();
}

static void saveStudents() {
    ofstream file("students.txt");
    for (auto it = studentsDB.begin(); it != studentsDB.end(); ++it) {
        const string& id = it->first;
        const Student& student = it->second;
        file << student.id << "," << student.name;
        for (size_t i = 0; i < student.registeredCourses.size(); ++i)
            file << "," << student.registeredCourses[i];
        file << endl;
    }
    file.close();
}

// ---------------------- Core Functionalities ----------------------

static bool hasPrerequisites(const Student& student, const Course& course) {
    unordered_set<string> completed;

    for (size_t i = 0; i < student.grades.size(); ++i)
        completed.insert(student.grades[i].courseCode);

    
    for (size_t i = 0; i < course.prerequisites.size(); ++i) {
        if (completed.find(course.prerequisites[i]) == completed.end()) {
            cout << "Missing prerequisite: " << course.prerequisites[i] << endl;
            return false; 
        }
    }
    return true; 
}


static void registerCourse(Student& student) {
    string code;
    cout << "Enter course code: ";
    cin >> code;

    if (courseCatalog.find(code) == courseCatalog.end()) {
        cout << "Course not found.\n";
        return;
    }

    if (find(student.registeredCourses.begin(), student.registeredCourses.end(), code) != student.registeredCourses.end()) {
        cout << "You have already registered for this course.\n";
        return;
    }

    Course& c = courseCatalog[code];

    if (!hasPrerequisites(student, c)) {
        cout << "Prerequisites not satisfied.\n";
        return;
    }

    student.registeredCourses.push_back(code);
    cout << "Registered successfully.\n";
}


static double gradeToGPA(const string& grade) {
    if (grade == "A") return 4.0;
    if (grade == "B") return 3.0;
    if (grade == "C") return 2.0;
    if (grade == "D") return 1.0;
    if (grade == "F") return 0.0;
    return -1.0; 
}

static void viewGrades(const Student& student) {
    for (size_t i = 0; i < student.grades.size(); ++i) {
        const GradeEntry& g = student.grades[i];
        cout << g.courseCode << " - Semester: " << g.semester << " - Grade: " << g.grade << endl;
    }
}

static void makeReport(const Student& student) {
    cout << "Student Name: " << student.name << ", ID: " << student.id << endl;

    double totalPoints = 0.0;
    int totalCredits = 0;

    for (const auto& g : student.grades) {
        if (courseCatalog.find(g.courseCode) == courseCatalog.end()) continue;

        const Course& c = courseCatalog[g.courseCode];
        double gpa = gradeToGPA(g.grade);

        if (gpa < 0) continue; // Skip invalid grade

        cout << g.courseCode << ", Semester: " << g.semester << ", Grade: " << g.grade
            << ", Credit: " << c.credits << ", GPA: " << gpa << endl;

        totalPoints += gpa * c.credits;
        totalCredits += c.credits;
    }

    double gpaResult = (totalCredits > 0) ? (totalPoints / totalCredits) : 0.0;
    cout << "Overall GPA: " << gpaResult << endl;
}
static void viewAvailableCourses(const Student& student) {
    cout << "Available Courses:\n";
    bool found = false;

    for (const auto& pair : courseCatalog) {
        const Course& c = pair.second;

        if (find(student.registeredCourses.begin(), student.registeredCourses.end(), c.code) != student.registeredCourses.end()) {
            cout << "Skipping " << c.code << " - Already registered\n";
            continue;
        }

        if (!hasPrerequisites(student, c)) {
            cout << "Skipping " << c.code << " - Prerequisites not met\n";
            continue;
        }

        cout << "Code: " << c.code
            << ", Title: " << c.title
            << ", Credits: " << c.credits
            << ", Instructor: " << c.instructor << endl;

        found = true;
    }

    if (!found) {
        cout << "No available courses found.\n";
    }
}



// ---------------------- Admin Functions ----------------------

static void uploadCourse() {
    Course c;
    cout << "Course code: "; cin >> c.code;
    cout << "Title: "; cin.ignore(); getline(cin, c.title);
    cout << "Syllabus: "; getline(cin, c.syllabus);
    cout << "Credits: "; cin >> c.credits;
    cout << "Instructor: "; cin.ignore(); getline(cin, c.instructor);
    cout << "Enter prerequisite course codes (comma separated): ";
    string preqStr; getline(cin, preqStr);
    stringstream ss(preqStr); string temp;
    while (getline(ss, temp, ',')) {
        if (!temp.empty())
            c.prerequisites.push_back(temp);
    }

    courseCatalog[c.code] = c;
    saveCourses(); 
    cout << "Course uploaded and saved.\n";
}


// ---------------------- Authentication ----------------------

static bool studentLogin(Student*& ptr) {
    string id;
    cout << "Enter student ID: ";
    cin >> id;
    if (studentsDB.find(id) != studentsDB.end()) {
        ptr = &studentsDB[id];
        return true;
    }
    cout << "Student not found.\n";
    return false;
}


static bool adminLogin(Admin*& ptr) {
    string user, pass;
    cout << "Enter username: "; cin >> user;
    cout << "Enter password: "; cin >> pass;
    if (adminsDB.find(user) != adminsDB.end() && adminsDB[user].password == pass) {
        ptr = &adminsDB[user];
        return true;
    }
    cout << "Admin not found or incorrect password.\n";
    return false;
}

static void loadAdmins() {
    ifstream file("admins.txt");
    string line;
    while (getline(file, line)) {
        Admin a;
        stringstream ss(line);
        getline(ss, a.username, ',');
        getline(ss, a.password, ',');
        adminsDB[a.username] = a;
    }
    file.close();
}
static void saveAdmins() {
    ofstream file("admins.txt");
    for (auto& kv : adminsDB) {
        file << kv.second.username << "," << kv.second.password << endl;
    }
    file.close();
}
static void loadGrades() {
    ifstream file("grades.txt");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string id, course, grade;
        int sem;
        getline(ss, id, ',');
        getline(ss, course, ',');
        getline(ss, grade, ',');
        ss >> sem;
        GradeEntry g = { course, grade, sem };
        studentsDB[id].grades.push_back(g);
    }
    file.close();
}
static void addStudent() {
    Student s;
    cout << "Enter student ID: ";
    cin >> s.id;
    if (studentsDB.find(s.id) != studentsDB.end()) {
        cout << "Student already exists.\n";
        return;
    }
    cout << "Enter student name: ";
    cin.ignore();
    getline(cin, s.name);
    studentsDB[s.id] = s;

    saveStudents(); 

    cout << "Student added successfully.\n";
}
static void saveGrades() {
    ofstream file("grades.txt");
    for (auto& pair : studentsDB) {
        const Student& s = pair.second;
        for (const auto& g : s.grades) {
            file << s.id << "," << g.courseCode << "," << g.grade << "," << g.semester << endl;
        }
    }
    file.close();
}
static void addOrUpdateGrade() {
    string studentId, courseCode, grade;
    int semester;

    cout << "Enter student ID: ";
    cin >> studentId;

    if (studentsDB.find(studentId) == studentsDB.end()) {
        cout << "Student not found.\n";
        return;
    }

    cout << "Enter course code: ";
    cin >> courseCode;

    cout << "Enter grade: ";
    cin >> grade;

    cout << "Enter semester: ";
    cin >> semester;

    
    bool gradeFound = false;
    stringstream updatedGrades;  

    ifstream file("grades.txt");
    string line;
    bool updated = false;

   
    while (getline(file, line)) {
        stringstream ss(line);
        string id, course, g;
        int sem;
        getline(ss, id, ',');
        getline(ss, course, ',');
        getline(ss, g, ',');
        ss >> sem;

        if (id == studentId && course == courseCode) {
            g = grade; 
            sem = semester; 
            updated = true;
        }

        updatedGrades << id << "," << course << "," << g << "," << sem << endl;
    }

    file.close();

    if (!updated) {
        updatedGrades << studentId << "," << courseCode << "," << grade << "," << semester << endl;
    }

    ofstream outFile("grades.txt");
    outFile << updatedGrades.str();  
    outFile.close();

    cout << "Grade added/updated successfully.\n";
}


static void enterGrade() {
    string id, course, grade;
    int semester;

    cout << "Enter student ID: ";
    cin >> id;

    if (studentsDB.find(id) == studentsDB.end()) {
        cout << "Student not found.\n";
        return;
    }

    cout << "Enter course code: ";
    cin >> course;

    if (courseCatalog.find(course) == courseCatalog.end()) {
        cout << "Course not found.\n";
        return;
    }

    cout << "Enter grade (A/B/C/...): ";
    cin >> grade;

    cout << "Enter semester number: ";
    cin >> semester;

    GradeEntry entry = { course, grade, semester };
    studentsDB[id].grades.push_back(entry);

    saveGrades(); 

    cout << "Grade recorded successfully.\n";
}

// ---------------------- Main Program ----------------------

int main() {
    // Load data once at start
    loadCourses();
    loadStudents();
    
    loadGrades();
    loadAdmins();
  

    int role;
    cout << "1. Student\n2. Admin\nEnter role: ";
    cin >> role;

    if (role == 1) {
        Student* stu;
        if (!studentLogin(stu)) return 0;
        int ch;
        do {
            cout << "1. Register Course\n2. View Grades\n3. Make Report\n4. View Available Courses\n0. Exit\nChoice: ";
            cin >> ch;
            switch (ch) {
            case 1: registerCourse(*stu); break;
            case 2: viewGrades(*stu); break;
            case 3: makeReport(*stu); break;
            case 4: viewAvailableCourses(*stu); break; 
            }
        } while (ch != 0);
    }
    else if (role == 2) {
        Admin* adm;
        if (!adminLogin(adm)) return 0;
        int ch;
        do {
            cout << "1. Upload Course\n2. Add Student\n3. Add/Update Grade\n0. Exit\nChoice: ";
            cin >> ch;
            switch (ch) {
            case 1: uploadCourse(); break;
            case 2: addStudent(); break;
            case 3: addOrUpdateGrade(); break;
            }
        } while (ch != 0);
    }
    
    // Save data once at end
    saveCourses();
    saveStudents();

    return 0;
}



