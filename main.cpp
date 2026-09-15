#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <random>

int randint(int a, int b) {
    static std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distr(a, b);
    int number =  distr(gen);
    return number;
}

std::string multiply_string(const std::string& string, int times) {
    std::string result;
    for (int i = 0; i < times; i++)
        result += string;
    return result;
}

struct Subject {
    std::string name;

    Subject(std::string name) : name(std::move(name)) {}
};
struct Teacher {
    std::string name;
    std::vector<Subject> qualifications;

    Teacher(std::string name, std::vector<Subject> qualifications) : name(std::move(name)), qualifications(std::move(qualifications)) {}
};

struct Requirements {
    int days = 5;
    std::vector<std::shared_ptr<Subject>> subjects = {
        std::make_shared<Subject>("Math"),
        std::make_shared<Subject>("DNA engineering"),
        std::make_shared<Subject>("Reverse-engineering"),
        std::make_shared<Subject>("Defrosting"),
        std::make_shared<Subject>("Kernel development"),
    };
    std::vector<std::shared_ptr<Teacher>> teachers = {
        std::make_shared<Teacher>("Cirpa Ilatera", std::vector<Subject>()),
        std::make_shared<Teacher>("Benjure", std::vector<Subject>()),
        std::make_shared<Teacher>("Guobian", std::vector<Subject>()),
        std::make_shared<Teacher>("Teleportan", std::vector<Subject>()),
    };
    struct SchoolClass {
        std::map<std::shared_ptr<Subject>, int> max_daily_load;
        std::map<std::shared_ptr<Subject>, int> min_weekly_load;

        SchoolClass(std::map<std::shared_ptr<Subject>, int> max_daily_load, std::map<std::shared_ptr<Subject>, int> min_weekly_load) 
            : max_daily_load(std::move(max_daily_load)), min_weekly_load(std::move(min_weekly_load)) {}
    };
    std::map<std::string, SchoolClass> classes;
};

struct Schedule {
    struct SchoolClass {
        struct Day {
            struct Lesson {
                std::shared_ptr<Subject> subject;
                std::shared_ptr<Teacher> teacher;

                Lesson(std::shared_ptr<Subject> subject, std::shared_ptr<Teacher> teacher) : subject(subject), teacher(teacher) {}
                void swap(Lesson& other) {
                    subject.swap(other.subject);
                    teacher.swap(other.teacher);
                }
            };
            std::vector<Lesson> lessons;

            Day(std::vector<Lesson> lessons) : lessons(std::move(lessons)) {}
            Day() {}
        };

        std::string name;
        std::vector<Day> schedule;

        SchoolClass(std::string name) : name(std::move(name)) {}
        SchoolClass(std::string name, std::vector<Day> schedule) : name(std::move(name)), schedule(std::move(schedule)) {}
        std::string string(int indentation, int index) {
            std::string result = multiply_string(" ", indentation * index) + name + " : {\n";
            for (int j = 0; j < schedule.size(); j++) {
                Day day = schedule.at(j);
                result += multiply_string(" ", (index + 1) * indentation) + std::to_string(j + 1) + ". {\n";
                for (int i = 0; i < day.lessons.size(); i++) {
                    auto lesson = day.lessons.at(i);
                    result += multiply_string(" ", (index + 2) * indentation) + std::to_string(i + 1) + ". " + lesson.subject->name + " - " + lesson.teacher->name;
                    if (i + 1 != day.lessons.size())
                        result += "\n";
                }
                result += "\n" + multiply_string(" ", (index + 1) * indentation) + "}";
                if (j + 1 != schedule.size())
                    result += "\n";
            }
            result += "\n" + multiply_string(" ", indentation * index) + "}";
            return result;
        }
    };

    std::vector<SchoolClass> classes;


    Schedule() {}
    std::string string() {
        std::string result;
        for (int i = 0; i < classes.size(); i++) {
            result += classes.at(i).string(4, 0);
            if (i + 1 != classes.size())
                result += ",\n";
        }
        return result;
    }
    float calculate_inaccuracy(const Requirements& requirements) {
        struct TeacherLoad {
            struct Day {
                std::vector<int> lessons;
            };
            std::vector<Day> days;

            TeacherLoad() {}
            TeacherLoad(std::vector<Day> days) : days(std::move(days)) {}
        };
        std::map<std::shared_ptr<Teacher>, TeacherLoad> teacher_load;
        float inaccuracy = 0.0;
        for (auto& school_class : classes) {
            int subjects_per_week = 0;
            for (auto& subject : requirements.classes.at(school_class.name).min_weekly_load) {
                subjects_per_week += subject.second;
            }
            int max_subjects_per_day = std::ceil((float)subjects_per_week / school_class.schedule.size());
            for (int i = 0; i < school_class.schedule.size(); i++) {
                auto& day = school_class.schedule.at(i);
                // Calculate appropriate amount of subjects per day inaccuracy
                if (day.lessons.size() > max_subjects_per_day) {
                    inaccuracy += std::pow(day.lessons.size() - max_subjects_per_day, 1.5);
                }
                // Calculate maximum daily load inaccuracy
                // 1. count all subjects
                std::map<std::shared_ptr<Subject>, int> subjects;
                for (int j = 0; j < day.lessons.size(); j++) {
                    auto& lesson = day.lessons.at(j);
                    if (!subjects.contains(lesson.subject)) {
                        subjects.emplace(lesson.subject, 0);
                    }
                    subjects.at(lesson.subject) += 1;
                    // These shenanigans are for counting teacher load
                    if (!teacher_load.contains(lesson.teacher)) {
                        teacher_load.emplace(lesson.teacher, std::vector<TeacherLoad::Day>(i));
                    }
                    auto& teacher = teacher_load.at(lesson.teacher);
                    if (teacher.days.size() <= i) {
                        teacher.days.resize(i + 1);
                    }
                    if (teacher.days.at(i).lessons.size() <= j) {
                        teacher.days.at(i).lessons.resize(j + 1);
                    }
                    teacher.days.at(i).lessons.at(j)++;
                }
                // 2. actually calculate the inaccuracies for maximum daily load
                for (auto& subject : requirements.classes.at(school_class.name).max_daily_load) {
                    if (subjects.contains(subject.first)) {
                        if (subjects.at(subject.first) > subject.second) {
                            inaccuracy += std::pow(subjects.at(subject.first) - subject.second, 1.5);
                        }
                    }
                }
            }
        }

        // calculate teacher load
        for (auto& teacher : teacher_load) {
            for (auto& day : teacher.second.days) {
                for (auto& lesson : day.lessons) {
                    if (lesson > 1) {
                        inaccuracy += std::pow(lesson, 2) ;
                    }
                }
            }
        }
        return inaccuracy;
    }
    void mutate(const Requirements& requirements) {
        int chance = randint(0, 100);
        
        // TODO  fix and refactor this
        auto& school_class = classes.at(randint(0, classes.size() - 1));
        int day_index = randint(0, school_class.schedule.size() - 1);
        for (int i = 0; i < school_class.schedule.size(); i++) {
            if (school_class.schedule.at(day_index).lessons.size() != 0)
                break;
            day_index = i;
        }
        auto& first_random_day = school_class.schedule.at(day_index);
        if (chance < 80) {
            // Swap random lessons
            auto& second_random_day = school_class.schedule.at(randint(0, school_class.schedule.size() - 1));
            if (first_random_day.lessons.size() == 0 || second_random_day.lessons.size() == 0)
                return;
            first_random_day.lessons.at(randint(0, first_random_day.lessons.size() - 1)).swap(second_random_day.lessons.at(randint(0, second_random_day.lessons.size() - 1)));
        } else if (chance < 90) {
            // Move a random lesson
            if (first_random_day.lessons.size() == 0)
                return;
            int lesson_index = randint(0, first_random_day.lessons.size() - 1);
            auto& second_random_day = school_class.schedule.at(randint(0, school_class.schedule.size() - 1));
            int index = 0;
            if (second_random_day.lessons.size() != 0)
                index = randint(0, second_random_day.lessons.size() - 1);
            second_random_day.lessons.insert(second_random_day.lessons.begin() + index, first_random_day.lessons.at(lesson_index));
            first_random_day.lessons.erase(first_random_day.lessons.begin() + lesson_index);
        } else {
            // Swap teacher
            if (first_random_day.lessons.size() == 0)
                return;
            first_random_day.lessons.at(randint(0, first_random_day.lessons.size() - 1)).teacher = requirements.teachers.at(randint(0, requirements.teachers.size() - 1));
        }
    }
};

Schedule build_schedule(const Requirements& requirements) {
    Schedule default_schedule;
    for (auto& school_class : requirements.classes) {
        default_schedule.classes.emplace_back(school_class.first);
        auto& class_ref = default_schedule.classes.back();
        for (int i = 0; i < requirements.days; i++)
            class_ref.schedule.emplace_back();
        int k = 0;
        for (auto& subject : school_class.second.min_weekly_load) {
            for (int i = 0; i < subject.second; i++) {
                int index = k % class_ref.schedule.size();
                class_ref.schedule.at(index).lessons.emplace_back(subject.first, requirements.teachers.at(randint(0, requirements.teachers.size() - 1)));
                k++;
            }
        }
    }
    std::cout << default_schedule.string() << std::endl;
    std::cout << default_schedule.calculate_inaccuracy(requirements) << std::endl;
    std::vector<Schedule> schedules;
    const int models = 100;
    const int max_training_cycles = 10000;
    const float survivors = 0.1;
    for (int i = 0; i < models; i++) {
        schedules.emplace_back(default_schedule);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
        schedules.back().mutate(requirements);
    }
    for (int i = 0; i < max_training_cycles; i++) {
        // Calculate inaccuracies
        std::vector<std::pair<float, Schedule>> rated_schedules;
        for (auto& schedule : schedules) {
            float inaccuracy = schedule.calculate_inaccuracy(requirements);
            rated_schedules.emplace_back(inaccuracy, std::move(schedule));
        }
        std::sort(rated_schedules.begin(), rated_schedules.end(), [](const std::pair<float, Schedule>& a, const std::pair<float, Schedule>& b){
            return a.first < b.first;
        });
        std::cout << "Best=" << rated_schedules.front().first << std::endl;
        // Check for solution
        if (rated_schedules.front().first == 0)
            return rated_schedules.front().second;
        // Eliminate fatalities
        for (int k = 0; k < models * (1 - survivors); k++) {
            rated_schedules.erase(rated_schedules.end() - 1);
        }
        // Reproduce
        schedules.clear();
        for (auto& survivor : rated_schedules) {
            schedules.emplace_back(survivor.second);
        }
        for (int k = 0;; k++) {
            if (schedules.size() >= models)
                break;
            schedules.emplace_back(rated_schedules.at(k % rated_schedules.size()).second);
            schedules.back().mutate(requirements);
            schedules.back().mutate(requirements);
        }
    }
    return schedules.back();
}

int main() {
    std::vector<std::shared_ptr<Subject>> subjects = {
        std::make_shared<Subject>("Math"),
        std::make_shared<Subject>("DNA engineering"),
        std::make_shared<Subject>("Reverse-engineering"),
        std::make_shared<Subject>("Defrosting"),
        std::make_shared<Subject>("Kernel development"),
    };
    std::vector<std::shared_ptr<Teacher>> teachers = {
        std::make_shared<Teacher>("Cirpa Ilatera", std::vector<Subject>()),
        std::make_shared<Teacher>("Lohman", std::vector<Subject>()),
        std::make_shared<Teacher>("Rof Jinkul", std::vector<Subject>()),
        std::make_shared<Teacher>("Han", std::vector<Subject>()),
        std::make_shared<Teacher>("Peter Walshere", std::vector<Subject>()),
        std::make_shared<Teacher>("Long Gregory", std::vector<Subject>()),
        std::make_shared<Teacher>("Patrick Mk II", std::vector<Subject>()),
        std::make_shared<Teacher>("Golden man", std::vector<Subject>()),
        std::make_shared<Teacher>("Benjure", std::vector<Subject>()),
        std::make_shared<Teacher>("Guobian", std::vector<Subject>()),
        std::make_shared<Teacher>("Teleportan", std::vector<Subject>()),
std::make_shared<Teacher>("Kira Shadow", std::vector<Subject>()),

// Mononyms / Last Names Only
std::make_shared<Teacher>("Garrison", std::vector<Subject>()),
std::make_shared<Teacher>("Kovacs", std::vector<Subject>()),
std::make_shared<Teacher>("Sterling", std::vector<Subject>()),
std::make_shared<Teacher>("Chen", std::vector<Subject>()),
std::make_shared<Teacher>("Vogel", std::vector<Subject>()),
std::make_shared<Teacher>("Blackwood", std::vector<Subject>()),
std::make_shared<Teacher>("Cross", std::vector<Subject>()),
std::make_shared<Teacher>("Dupont", std::vector<Subject>()),
std::make_shared<Teacher>("Evans", std::vector<Subject>()),
std::make_shared<Teacher>("Fisher", std::vector<Subject>()),
std::make_shared<Teacher>("Grant", std::vector<Subject>()),
std::make_shared<Teacher>("Hayes", std::vector<Subject>()),
std::make_shared<Teacher>("Ivers", std::vector<Subject>()),
std::make_shared<Teacher>("Jones", std::vector<Subject>()),
std::make_shared<Teacher>("Kingsley", std::vector<Subject>()),

// Additional Unique Names
std::make_shared<Teacher>("Alistair Finch", std::vector<Subject>()),
std::make_shared<Teacher>("Beatrix Potter", std::vector<Subject>()),
std::make_shared<Teacher>("Caspian North", std::vector<Subject>()),
std::make_shared<Teacher>("Dorothy Gale", std::vector<Subject>()),
std::make_shared<Teacher>("Ethan Hunt", std::vector<Subject>()),
std::make_shared<Teacher>("Freya Stark", std::vector<Subject>()),
std::make_shared<Teacher>("Gideon Grey", std::vector<Subject>()),
std::make_shared<Teacher>("Hazel Wood", std::vector<Subject>()),
std::make_shared<Teacher>("Iris West", std::vector<Subject>()),
std::make_shared<Teacher>("Jasper Hale", std::vector<Subject>()),
std::make_shared<Teacher>("Katniss Everdeen", std::vector<Subject>()),
std::make_shared<Teacher>("Logan Howlett", std::vector<Subject>()),
std::make_shared<Teacher>("Mildred Pierce", std::vector<Subject>()),
std::make_shared<Teacher>("Nathan Drake", std::vector<Subject>()),
std::make_shared<Teacher>("Ophelia Crane", std::vector<Subject>())
    };
    Requirements requirements;
    requirements.teachers = std::move(teachers);
    for (int i = 0; i < 50; i++) {
        requirements.classes.emplace(std::string(std::to_string(i) + "B"),
            Requirements::SchoolClass({std::map<std::shared_ptr<Subject>, int>({
                {subjects.at(0), 2},
                {subjects.at(1), 2},
                {subjects.at(2), 2},
                {subjects.at(3), 2},
                {subjects.at(4), 3},
                }),
            std::map<std::shared_ptr<Subject>, int>({
                {subjects.at(0), 10},
                {subjects.at(1), 6},
                {subjects.at(2), 7},
                {subjects.at(3), 5},
                {subjects.at(4), 10},
                })})
        );
    }
    auto built_schedule = build_schedule(requirements);
    std::cout << built_schedule.string() << "\n";
    std::cout << built_schedule.calculate_inaccuracy(requirements);
    return 0;
}
