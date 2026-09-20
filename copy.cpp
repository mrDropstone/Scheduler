#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <numbers>
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
    Teacher(std::string name) : name(std::move(name)) {}
    bool operator==(const Teacher& other) {
        return name == other.name;
    }
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
        std::map<std::shared_ptr<Subject>, std::shared_ptr<Teacher>> teachers;

        SchoolClass(std::map<std::shared_ptr<Subject>, int> max_daily_load, std::map<std::shared_ptr<Subject>, int> min_weekly_load, 
                std::map<std::shared_ptr<Subject>, std::shared_ptr<Teacher>> teachers) 
            : max_daily_load(std::move(max_daily_load)), min_weekly_load(std::move(min_weekly_load)), teachers(std::move(teachers)) {}
    };
    std::map<std::string, SchoolClass> classes;
};

struct Schedule {
    struct Lesson {
        std::shared_ptr<Subject> subject;
        std::shared_ptr<Teacher> teacher;

        Lesson(std::shared_ptr<Subject> subject, std::shared_ptr<Teacher> teacher) : subject(subject), teacher(teacher) {}
        void swap(Lesson& other) {
            subject.swap(other.subject);
            teacher.swap(other.teacher);
        }
    };
    struct Day {
        std::vector<Lesson> lessons;

        Day(std::vector<Lesson> lessons) : lessons(std::move(lessons)) {}
        Day() {}
    };
    struct SchoolClass {

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
    struct TeacherLoad {
        struct Day {
            std::vector<int> lessons{15};
        };
        std::vector<Day> days;

        TeacherLoad() {}
        TeacherLoad(std::vector<Day> days) : days(std::move(days)) {}
    };
    std::map<std::shared_ptr<Teacher>, TeacherLoad> teacher_load() {
        std::map<std::shared_ptr<Teacher>, TeacherLoad> teacher_load;
        for (auto& school_class : classes) {
            for (int i = 0; i < school_class.schedule.size(); i++) {
                auto& day = school_class.schedule.at(i);
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
            }
        }
        int k = 0;
        std::map<std::shared_ptr<Teacher>, TeacherLoad> only_teachers_with_lessons;
        for (auto& teacher : teacher_load) {
            bool has_lessons = false;
            for (auto& day : teacher.second.days) {
                for (auto& lesson : day.lessons) {
                    if (lesson != 0) {
                        has_lessons = true;
                        break;
                    }
                }
            }
            if (has_lessons) {
                only_teachers_with_lessons.emplace(std::move(teacher));
            }
        }
        return only_teachers_with_lessons;
    }
    float calculate_inaccuracy(const Requirements& requirements) {
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
                    //inaccuracy += std::pow(day.lessons.size() - max_subjects_per_day, 1.5);
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
                            //inaccuracy += std::pow(subjects.at(subject.first) - subject.second, 1.5);
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
                        inaccuracy += std::pow(lesson, 1) * 1000;
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
    const int max_training_cycles = 100000;
    const float survivors = 0.05;
    for (int i = 0; i < models; i++) {
        schedules.emplace_back(default_schedule);
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
        }
    }
    return schedules.back();
}

template <typename T>
constexpr T& get_random(std::vector<T>& vector) {
    assert(vector.size() != 0);
    return vector.at(randint(0, vector.size() - 1));
}

Schedule create_schedule(const Requirements& requirements) {
    Schedule schedule;
    /*
    schedule.classes.emplace_back(Schedule::SchoolClass(
        "67SUI",
        std::vector<Schedule::Day>({
            Schedule::Day({Schedule::Lesson(requirements.subjects.at(0), requirements.teachers.at(0))})
        })));
    */
    for (auto& school_class : requirements.classes) {
        schedule.classes.emplace_back(school_class.first);
        auto& class_ref = schedule.classes.back();
        for (int i = 0; i < requirements.days; i++)
            class_ref.schedule.emplace_back();
        int k = randint(0, 67);
        for (auto& subject : school_class.second.min_weekly_load) {
            for (int i = 0; i < subject.second; i++) {
                int index = k % class_ref.schedule.size();
                class_ref.schedule.at(index).lessons.emplace_back(subject.first, school_class.second.teachers.at(subject.first));
                k++;
            }
        }
    }
    std::cout << schedule.string() << std::endl;
    auto teacher_load = schedule.teacher_load();
    int cycles = 1000000;
    float overall_score = schedule.calculate_inaccuracy(requirements);
    // Balance
    while (1) {
        std::pair<const std::shared_ptr<Teacher>, Schedule::TeacherLoad>* teacher_ptr1 = nullptr;
        std::pair<const std::shared_ptr<Teacher>, Schedule::TeacherLoad>* teacher_ptr2 = nullptr;
        int day_index1;
        int day_index2;
        int lesson_index1;
        int lesson_index2;
        for (auto& teacher : teacher_load) {
            for (int day = 0; day < teacher.second.days.size(); day++) {
                auto& day_ref = teacher.second.days.at(day);
                for (int lesson = 0; lesson < day_ref.lessons.size(); lesson++) {
                    int lesson_copy = day_ref.lessons.at(lesson);
                    if (lesson_copy > 1 && randint(0, 3) == 0) {
                        teacher_ptr1 = &teacher;
                        day_index1 = day;
                        lesson_index1 = lesson;
                        break;
                    }
                }
            }
        }
        if (teacher_ptr1 == nullptr) {
            continue;
        }
        for (auto& teacher : teacher_load) {
            if (teacher.second.days.size() <= day_index1) {
                continue;
            }
            if (teacher.second.days.at(day_index1).lessons.size() <= lesson_index1) {
                continue;
            }
            if (teacher.second.days.at(day_index1).lessons.at(lesson_index1) != 0) {
                continue;
            }
            for (int day = 0; day < teacher.second.days.size(); day++) {
                auto& day_ref = teacher.second.days.at(day);
                if (teacher_ptr1->second.days.size() <= day) {
                    continue;
                }
                for (int lesson = 0; lesson < day_ref.lessons.size(); lesson++) {
                    if (teacher_ptr1->second.days.at(day).lessons.size() <= lesson) {
                        continue;
                    }
                    if (teacher_ptr1->second.days.at(day).lessons.at(lesson) != 0 || day_ref.lessons.at(lesson) == 0) {
                        continue;
                    }
                    int lesson_copy = day_ref.lessons.at(lesson);
                    teacher_ptr2 = &teacher;
                    day_index2 = day;
                    lesson_index2 = lesson;
                    break;
                }
            }
        }
        if (teacher_ptr2 == nullptr) {
            continue;
        }
        std::cout << schedule.calculate_inaccuracy(requirements) << std::endl;
        Schedule::Lesson* lesson_ptr1 = nullptr;
        Schedule::Lesson* lesson_ptr2 = nullptr;
        for (auto& school_class : schedule.classes) {
            if (day_index1 >= school_class.schedule.size() || day_index2 >= school_class.schedule.size()) {
                continue;
            }
            if (lesson_index1 >= school_class.schedule.at(day_index1).lessons.size() || lesson_index2 >= school_class.schedule.at(day_index2).lessons.size()) {
                continue;
            }
            if (school_class.schedule.at(day_index1).lessons.at(lesson_index1).teacher == teacher_ptr1->first) {
                lesson_ptr1 = &school_class.schedule.at(day_index1).lessons.at(lesson_index1);
            }
            if (school_class.schedule.at(day_index2).lessons.at(lesson_index2).teacher == teacher_ptr2->first) {
                lesson_ptr2 = &school_class.schedule.at(day_index2).lessons.at(lesson_index2);
            }
        }
        std::cout << lesson_ptr1 << std::endl;
        std::cout << lesson_ptr2 << std::endl;
        assert(lesson_ptr1 != nullptr && lesson_ptr2 != nullptr);
        lesson_ptr1->swap(*lesson_ptr2);
        std::cout << "anes" << std::endl;
        teacher_ptr1->second.days.at(day_index1).lessons.at(lesson_index1)--;
        teacher_ptr1->second.days.at(day_index1).lessons.at(lesson_index2)++;
        teacher_ptr1->second.days.at(day_index2).lessons.at(lesson_index1)++;
        teacher_ptr1->second.days.at(day_index2).lessons.at(lesson_index2)--;
        std::cout << "nes" << std::endl;
    }
    return schedule;
    float temperature = 2000;
    const float decay_constant = 0.99;
    const float decay2 = 1.0;
    for (int i = 0; i < cycles; i++) {
        int chance = randint(0, 100);
        // balance
        if (1) {
            std::pair<const std::shared_ptr<Teacher>, Schedule::TeacherLoad>* teacher_ptr1 = nullptr;
            std::pair<const std::shared_ptr<Teacher>, Schedule::TeacherLoad>* teacher_ptr2 = nullptr;
            int day_index1;
            int day_index2;
            int lesson_index1;
            int lesson_index2;
            for (auto& teacher : teacher_load) {
                for (int day = 0; day < teacher.second.days.size(); day++) {
                    auto& day_ref = teacher.second.days.at(day);
                    for (int lesson = 0; lesson < day_ref.lessons.size(); lesson++) {
                        int lesson_copy = day_ref.lessons.at(lesson);
                        if (lesson_copy == 0) {
                        } else if (lesson_copy > 1 && teacher_ptr2 != nullptr) {
                            day_index2 = day;
                            lesson_index2 = lesson;
                            teacher_ptr2 = &teacher;
                        } else if (lesson_copy > 1 && teacher_ptr1 != nullptr) {
                            teacher_ptr1 = &teacher;
                            day_index1 = day;
                            lesson_index1 = lesson;
                        }
                    }
                }
            }
            if (teacher_ptr1 == nullptr || teacher_ptr2 == nullptr) {
                break;
            }
            std::cout << "es" << std::endl;
            Schedule::Day* day_ptr1;
            Schedule::Day* day_ptr2;
            Schedule::Lesson* lesson_ptr1 = nullptr;
            Schedule::Lesson* lesson_ptr2 = nullptr;
            std::cout << teacher_ptr1->first->name << std::endl;
            for (auto& school_class : schedule.classes) {
                std::cout << school_class.schedule.at(day_index1).lessons.at(lesson_index1).teacher->name << std::endl;
                if (school_class.schedule.at(day_index1).lessons.at(lesson_index1).teacher == teacher_ptr1->first) {
                    lesson_ptr1 = &school_class.schedule.at(day_index1).lessons.at(lesson_index1);
                }
                if (school_class.schedule.at(day_index2).lessons.at(lesson_index2).teacher == teacher_ptr2->first) {
                    lesson_ptr2 = &school_class.schedule.at(day_index2).lessons.at(lesson_index2);
                }
            }
            assert(lesson_ptr1 != nullptr && lesson_ptr2 != nullptr);
            lesson_ptr1->swap(*lesson_ptr2);
            std::cout << "anes" << std::endl;
            teacher_ptr1->second.days.at(day_index1).lessons.at(lesson_index1)++;
            teacher_ptr2->second.days.at(day_index2).lessons.at(lesson_index2)++;
            std::cout << "nes" << std::endl;
        }
        continue;
        auto& random_class = get_random(schedule.classes);
        int day_index1 = randint(0, random_class.schedule.size() - 1);
        int day_index2 = randint(0, random_class.schedule.size() - 1);
        auto& day = random_class.schedule.at(day_index1);
        auto& day2 = random_class.schedule.at(day_index2);
        if (day.lessons.size() == 0 || day2.lessons.size() == 0) {
            continue;
        };
        int lesson_index1 = randint(0, day.lessons.size() - 1);
        int lesson_index2 = randint(0, day2.lessons.size() - 1);
        auto& teacher1 = teacher_load.at(day.lessons.at(lesson_index1).teacher);
        auto& teacher2 = teacher_load.at(day2.lessons.at(lesson_index2).teacher);
        day.lessons.at(lesson_index1).swap(day2.lessons.at(lesson_index2));
        float score = schedule.calculate_inaccuracy(requirements);
        float difference = score - overall_score;
        if (score == 0.0) {
            return schedule;
        }
        float acceptance_probability;
        if (temperature == 0) {
            acceptance_probability = 0;
        } else {
            acceptance_probability = std::pow(std::numbers::e, -difference / temperature);
        }
        std::cout << "temperature = " << temperature << std::endl;
        if (difference < 0.0 || acceptance_probability > randint(0, 100) / 100.0) {
            assert(day.lessons.size() > lesson_index1);
            assert(day2.lessons.size() > lesson_index2);
            overall_score = score;
        } else {
            day.lessons.at(lesson_index1).swap(day2.lessons.at(lesson_index2));
        }
        std::cout << overall_score << std::endl;
    }
    return schedule;
}

int main() {
    std::vector<std::shared_ptr<Subject>> subjects = {
        std::make_shared<Subject>("Math"),
        std::make_shared<Subject>("DNA engineering"),
        std::make_shared<Subject>("Reverse-engineering"),
        std::make_shared<Subject>("Defrosting"),
        std::make_shared<Subject>("Kernel development"),
        std::make_shared<Subject>("Reverse-grinding"),
        std::make_shared<Subject>("Cyberinsecurity"),
        std::make_shared<Subject>("Neurobiology"),
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
    };/*
        std::make_shared<Teacher>("Benjure", std::vector<Subject>()),
        std::make_shared<Teacher>("Guobian", std::vector<Subject>()),
        std::make_shared<Teacher>("Teleportan", std::vector<Subject>()),
        std::make_shared<Teacher>("Bobby Fisher", std::vector<Subject>()),
        std::make_shared<Teacher>("John Watching", std::vector<Subject>()),
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
        std::make_shared<Teacher>("Ophelia Crane", std::vector<Subject>()),
        std::make_shared<Teacher>("Penelope Vance", std::vector<Subject>()),
        std::make_shared<Teacher>("Quentin Quire", std::vector<Subject>()),
        std::make_shared<Teacher>("Rowena Raven", std::vector<Subject>()),
        std::make_shared<Teacher>("Silas Marner", std::vector<Subject>()),
        std::make_shared<Teacher>("Tabitha Twitchit", std::vector<Subject>()),
        std::make_shared<Teacher>("Ulysses Storm", std::vector<Subject>()),
        std::make_shared<Teacher>("Victor Frankenstein", std::vector<Subject>()),
        std::make_shared<Teacher>("Wendy Darling", std::vector<Subject>()),
        std::make_shared<Teacher>("Xavier Woods", std::vector<Subject>()),
        std::make_shared<Teacher>("Yvaine Stardust", std::vector<Subject>()),
        std::make_shared<Teacher>("Zachary Taylor", std::vector<Subject>()),
        std::make_shared<Teacher>("Arthur Pendragon", std::vector<Subject>()),
        std::make_shared<Teacher>("Clara Oswald", std::vector<Subject>()),
    };
    /*
        std::make_shared<Teacher>("Desmond Miles", std::vector<Subject>()),
        std::make_shared<Teacher>("Eleanor Vance", std::vector<Subject>()),
        std::make_shared<Teacher>("Felix Faust", std::vector<Subject>()),
        std::make_shared<Teacher>("Garrick Ollivander", std::vector<Subject>()),
        std::make_shared<Teacher>("Helena Ravenclaw", std::vector<Subject>()),
        std::make_shared<Teacher>("Julian Bashir", std::vector<Subject>()),
        std::make_shared<Teacher>("Katarina Rostova", std::vector<Subject>()),
        std::make_shared<Teacher>("Lucius Fox", std::vector<Subject>()),
        std::make_shared<Teacher>("Minerva McGonagall", std::vector<Subject>()),
        std::make_shared<Teacher>("Nesta Archeron", std::vector<Subject>()),
        std::make_shared<Teacher>("Oberon Martell", std::vector<Subject>()),
        std::make_shared<Teacher>("Phineas Fogg", std::vector<Subject>()),
        std::make_shared<Teacher>("Quincy Harker", std::vector<Subject>()),
        std::make_shared<Teacher>("Reginald Hargreeves", std::vector<Subject>()),
        std::make_shared<Teacher>("Severus Snape", std::vector<Subject>()),
        std::make_shared<Teacher>("Tristan Thorn", std::vector<Subject>()),
        std::make_shared<Teacher>("Ursula Buffay", std::vector<Subject>()),
        std::make_shared<Teacher>("Vivian Vance", std::vector<Subject>()),
        std::make_shared<Teacher>("Winston Smith", std::vector<Subject>()),
        std::make_shared<Teacher>("Xander Harris", std::vector<Subject>()),
        std::make_shared<Teacher>("Yennefer Vengerberg", std::vector<Subject>()),
        std::make_shared<Teacher>("Zelda Fitzgerald", std::vector<Subject>()),
        std::make_shared<Teacher>("Albus Dumbledore", std::vector<Subject>()),
        std::make_shared<Teacher>("Bruce Wayne", std::vector<Subject>()),
        std::make_shared<Teacher>("Cassandra Cain", std::vector<Subject>()),
        std::make_shared<Teacher>("Diana Prince", std::vector<Subject>()),
        std::make_shared<Teacher>("Edward Elric", std::vector<Subject>()),
        std::make_shared<Teacher>("Fox Mulder", std::vector<Subject>()),
        std::make_shared<Teacher>("Gilbert Blythe", std::vector<Subject>()),
        std::make_shared<Teacher>("Harvey Dent", std::vector<Subject>()),
        std::make_shared<Teacher>("Icabod Crane", std::vector<Subject>()),
        std::make_shared<Teacher>("Jack Torrance", std::vector<Subject>()),
        std::make_shared<Teacher>("Klaus Hargreeves", std::vector<Subject>()),
        std::make_shared<Teacher>("Lara Croft", std::vector<Subject>()),
        std::make_shared<Teacher>("Moira Rose", std::vector<Subject>()),
        std::make_shared<Teacher>("Newt Scamander", std::vector<Subject>()),
        std::make_shared<Teacher>("Oliver Queen", std::vector<Subject>()),
        std::make_shared<Teacher>("Peggy Carter", std::vector<Subject>()),
        std::make_shared<Teacher>("Remus Lupin", std::vector<Subject>()),
        std::make_shared<Teacher>("Sherlock Holmes", std::vector<Subject>()),
        std::make_shared<Teacher>("Thomas Wayne", std::vector<Subject>()),
        std::make_shared<Teacher>("Victor Stone", std::vector<Subject>()),
        std::make_shared<Teacher>("Abercrombie", std::vector<Subject>()),
        std::make_shared<Teacher>("Bancroft", std::vector<Subject>()),
        std::make_shared<Teacher>("Carlisle", std::vector<Subject>()),
        std::make_shared<Teacher>("Davenport", std::vector<Subject>()),
        std::make_shared<Teacher>("Ellington", std::vector<Subject>()),
        std::make_shared<Teacher>("Fitzgerald", std::vector<Subject>()),
        std::make_shared<Teacher>("Garrison", std::vector<Subject>()),
        std::make_shared<Teacher>("Hawthorne", std::vector<Subject>()),
        std::make_shared<Teacher>("Ingram", std::vector<Subject>()),
        std::make_shared<Teacher>("Kensington", std::vector<Subject>()),
        std::make_shared<Teacher>("Langdon", std::vector<Subject>()),
        std::make_shared<Teacher>("Montgomery", std::vector<Subject>()),
        std::make_shared<Teacher>("Nightingale", std::vector<Subject>()),
        std::make_shared<Teacher>("Pendleton", std::vector<Subject>()),
        std::make_shared<Teacher>("Quinn", std::vector<Subject>()),
        std::make_shared<Teacher>("Radcliffe", std::vector<Subject>()),
        std::make_shared<Teacher>("Sinclair", std::vector<Subject>()),
        std::make_shared<Teacher>("Thornton", std::vector<Subject>()),
        std::make_shared<Teacher>("Underwood", std::vector<Subject>()),
        std::make_shared<Teacher>("Vanderbilt", std::vector<Subject>()),
        std::make_shared<Teacher>("Wellington", std::vector<Subject>()),
        std::make_shared<Teacher>("Kingsley", std::vector<Subject>()),
        std::make_shared<Teacher>("Xavier", std::vector<Subject>()),
        std::make_shared<Teacher>("Yardley", std::vector<Subject>()),
        std::make_shared<Teacher>("Zimmerman", std::vector<Subject>()),
        std::make_shared<Teacher>("Albert Camus", std::vector<Subject>()),
        std::make_shared<Teacher>("Arthur Conan", std::vector<Subject>()),
        std::make_shared<Teacher>("Bram Stoker", std::vector<Subject>()),
        std::make_shared<Teacher>("Charles Dickens", std::vector<Subject>()),
        std::make_shared<Teacher>("Edgar Poe", std::vector<Subject>()),
        std::make_shared<Teacher>("Ernest Hemingway", std::vector<Subject>()),
        std::make_shared<Teacher>("Franz Kafka", std::vector<Subject>()),
        std::make_shared<Teacher>("George Orwell", std::vector<Subject>()),
        std::make_shared<Teacher>("Herman Melville", std::vector<Subject>()),
        std::make_shared<Teacher>("Isaac Asimov", std::vector<Subject>()),
        std::make_shared<Teacher>("Jane Austen", std::vector<Subject>()),
        std::make_shared<Teacher>("Leo Tolstoy", std::vector<Subject>()),
        std::make_shared<Teacher>("Mark Twain", std::vector<Subject>()),
        std::make_shared<Teacher>("Oscar Wilde", std::vector<Subject>()),
        std::make_shared<Teacher>("Ray Bradbury", std::vector<Subject>()),
        std::make_shared<Teacher>("Sylvia Plath", std::vector<Subject>()),
        std::make_shared<Teacher>("Virginia Woolf", std::vector<Subject>()),
        std::make_shared<Teacher>("Walter Scott", std::vector<Subject>()),
        std::make_shared<Teacher>("Aldous Huxley", std::vector<Subject>()),
        std::make_shared<Teacher>("Agatha Christie", std::vector<Subject>()),
        std::make_shared<Teacher>("F. Scott Fitzgerald", std::vector<Subject>()),
        std::make_shared<Teacher>("Mary Shelley", std::vector<Subject>()),
        std::make_shared<Teacher>("J.R.R. Tolkien", std::vector<Subject>()),
        std::make_shared<Teacher>("C.S. Lewis", std::vector<Subject>()),
        std::make_shared<Teacher>("Lewis Carroll", std::vector<Subject>()),
        std::make_shared<Teacher>("Arthur Dent", std::vector<Subject>()),
        std::make_shared<Teacher>("Clark Kent", std::vector<Subject>()),
        std::make_shared<Teacher>("Dana Scully", std::vector<Subject>()),
        std::make_shared<Teacher>("Ellen Ripley", std::vector<Subject>()),
        std::make_shared<Teacher>("Ford Prefect", std::vector<Subject>()),
        std::make_shared<Teacher>("Gwen Stacy", std::vector<Subject>()),
        std::make_shared<Teacher>("Hal Jordan", std::vector<Subject>()),
        std::make_shared<Teacher>("Indiana Jones", std::vector<Subject>()),
        std::make_shared<Teacher>("James Bond", std::vector<Subject>()),
        std::make_shared<Teacher>("Katara Water", std::vector<Subject>()),
        std::make_shared<Teacher>("Luke Skywalker", std::vector<Subject>()),
        std::make_shared<Teacher>("Marty McFly", std::vector<Subject>()),
        std::make_shared<Teacher>("Neo Anderson", std::vector<Subject>()),
        std::make_shared<Teacher>("Peter Parker", std::vector<Subject>()),
        std::make_shared<Teacher>("Sarah Connor", std::vector<Subject>()),
        std::make_shared<Teacher>("Tony Stark", std::vector<Subject>()),
        std::make_shared<Teacher>("Wanda Maximoff", std::vector<Subject>()),
        std::make_shared<Teacher>("Zaphod Beeblebrox", std::vector<Subject>()),
        std::make_shared<Teacher>("Katniss Mellark", std::vector<Subject>()),
        std::make_shared<Teacher>("Jon Snow", std::vector<Subject>()),
        std::make_shared<Teacher>("Arya Stark", std::vector<Subject>()),
        std::make_shared<Teacher>("Tyrion Lannister", std::vector<Subject>()),
        std::make_shared<Teacher>("Daenerys Targaryen", std::vector<Subject>()),
        std::make_shared<Teacher>("Barry Allen", std::vector<Subject>()),
        std::make_shared<Teacher>("Bruce Banner", std::vector<Subject>()),
        std::make_shared<Teacher>("Asher Vance", std::vector<Subject>()),
        std::make_shared<Teacher>("Blair Waldorf", std::vector<Subject>()),
        std::make_shared<Teacher>("Cole Sear", std::vector<Subject>()),
        std::make_shared<Teacher>("Damon Salvatore", std::vector<Subject>()),
        std::make_shared<Teacher>("Elena Gilbert", std::vector<Subject>()),
        std::make_shared<Teacher>("Fiona Gallagher", std::vector<Subject>()),
        std::make_shared<Teacher>("Grant Ward", std::vector<Subject>()),
        std::make_shared<Teacher>("Holden Caulfield", std::vector<Subject>()),
        std::make_shared<Teacher>("Ivy Pepper", std::vector<Subject>()),
        std::make_shared<Teacher>("Jax Teller", std::vector<Subject>()),
        std::make_shared<Teacher>("Klaus Mikaelson", std::vector<Subject>()),
        std::make_shared<Teacher>("Lucas Scott", std::vector<Subject>()),
        std::make_shared<Teacher>("Meredith Grey", std::vector<Subject>()),
        std::make_shared<Teacher>("Nathan Scott", std::vector<Subject>()),
        std::make_shared<Teacher>("Owen Hunt", std::vector<Subject>()),
        std::make_shared<Teacher>("Piper Halliwell", std::vector<Subject>()),
        std::make_shared<Teacher>("Quinn Fabray", std::vector<Subject>()),
        std::make_shared<Teacher>("Ray Palmer", std::vector<Subject>()),
        std::make_shared<Teacher>("Spencer Hastings", std::vector<Subject>()),
        std::make_shared<Teacher>("Tristan Dugray", std::vector<Subject>()),
        std::make_shared<Teacher>("Umaru Doma", std::vector<Subject>()),
        std::make_shared<Teacher>("Victor Nikiforov", std::vector<Subject>()),
        std::make_shared<Teacher>("Wyatt Halliwell", std::vector<Subject>()),
        std::make_shared<Teacher>("Zoe Hart", std::vector<Subject>()),
        std::make_shared<Teacher>("Zane Truesdale", std::vector<Subject>())
    };*/
    Requirements requirements;
    requirements.teachers = (teachers);
    for (int j = 0; j < 20; j++) {
        int i = 0;
        requirements.classes.emplace(std::string(std::to_string(j) + "B"),
            Requirements::SchoolClass({std::map<std::shared_ptr<Subject>, int>({
                {subjects.at(0), 2},
                {subjects.at(1), 2},
                {subjects.at(2), 2},
                {subjects.at(3), 2},
                {subjects.at(4), 3},
                {subjects.at(5), 5},
                {subjects.at(6), 4},
                }),
            std::map<std::shared_ptr<Subject>, int>({
                {subjects.at(0), 5},
                {subjects.at(1), 6},
                {subjects.at(2), 5},
                {subjects.at(3), 5},
                {subjects.at(4), 4},
                {subjects.at(5), 4},
                {subjects.at(6), 4},
                }),
            std::map<std::shared_ptr<Subject>, std::shared_ptr<Teacher>>({
                {subjects.at(0), teachers.at(i)},
                {subjects.at(1), teachers.at(i + 1)},
                {subjects.at(2), teachers.at(i + 2)},
                {subjects.at(3), teachers.at(i + 3)},
                {subjects.at(4), teachers.at(i + 4)},
                {subjects.at(5), teachers.at(i + 5)},
                {subjects.at(6), teachers.at(i + 6)},
                })
            })
        );
    }
    //auto built_schedule = build_schedule(requirements);
    for (int i = 0; i < 1; i++) {
        auto built_schedule = build_schedule(requirements);
        std::cout << built_schedule.calculate_inaccuracy(requirements) << "\n";
        //std::cout << built_schedule.string() << "\n";
    }
    return 0;
}
