#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numbers>
#include <string>
#include <utility>
#include <vector>
#include "utils.hpp"

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
struct Class {
    std::string name;
};

struct Requirements {
    struct SubjectRequirements {
        int min_weekly_load;
        int max_daily_load;
        std::shared_ptr<Teacher> teacher;
    };

    int days = 5;
    std::vector<std::shared_ptr<Subject>> subjects;
    std::vector<std::shared_ptr<Teacher>> teachers;
    std::map<std::shared_ptr<Class>, std::map<std::shared_ptr<Subject>, SubjectRequirements>> classes;
};




struct Prototype {
    struct Lesson {
        std::shared_ptr<Subject> subject;

        Lesson() {}
        Lesson(std::shared_ptr<Subject> subject) : subject(subject) {}
        void swap(Lesson& other) {
            subject.swap(other.subject);
        }
    };
    struct Day {
        std::vector<Lesson> lessons;

        Day(std::vector<Lesson> lessons) : lessons(std::move(lessons)) {}
        Day() {}
    };
    struct Genome {
        int lesson = 0;
        int day = 0;
        std::shared_ptr<Subject> subject;
        std::shared_ptr<Teacher> teacher;
        std::shared_ptr<Class> school_class;

        Genome() {}
        Genome(std::shared_ptr<Subject> subject, std::shared_ptr<Teacher> teacher, std::shared_ptr<Class> school_class) : subject(subject), teacher(teacher), school_class(school_class) {}
        std::string string() {
            return "day = " + std::to_string(day) + ", lesson = " + std::to_string(lesson)
                + ", subject = " + subject->name + ", teacher = " + teacher->name + ", class = " + school_class->name;
        }
    };
    struct Validation {
        int teacher_double_bookings = 0;
        int class_double_bookings = 0;
        int gaps = 0;
    };
    using Conflicts = std::vector<std::vector<int>>;

    std::map<std::shared_ptr<Class>, Conflicts> class_data;
    std::map<std::shared_ptr<Teacher>, Conflicts> teacher_data;
    std::vector<Genome> genomes;
    int score = 0;


    void mutate(const Requirements& requirements) {
        auto& genome = get_random(genomes);
        int new_day_index = randint(0, requirements.days - 1);
        int new_lesson_index = randint(0, 9);

        auto ensure_size = [](std::vector<std::vector<int>>& vector, int day, int lesson) {
            if (vector.size() <= day) {
                vector.resize(day + 1);
            }
            if (vector.at(day).size() <= lesson) {
                vector.at(day).resize(lesson + 1, 0);
            }
        };

        // Calculate class double bookings
        auto& school_class = class_data.at(genome.school_class);
        if (school_class.at(genome.day).at(genome.lesson) > 1) {
            score -= 20000;
        }
        ensure_size(school_class, new_day_index, new_lesson_index);
        if (school_class.at(new_day_index).at(new_lesson_index) > 0) {
            score += 20000;
        }

        // Calculate teacher double-bookings
        auto& teacher = teacher_data.at(genome.teacher);
        if (teacher.at(genome.day).at(genome.lesson) > 1) {
            score -= 20000;
        }
        ensure_size(teacher, new_day_index, new_lesson_index);
        if (teacher.at(new_day_index).at(new_lesson_index) > 0) {
            score += 20000;
        }

        // Calculate gaps
        auto calculate_gaps = [](const std::vector<std::vector<int>>& school_class, int day) {
            if (school_class.at(day).size() == 0) {
                return 0;
            }
            int k = school_class.at(day).size() - 1;
            while (k >= 0) {
                if (school_class.at(day).at(k) != 0) {
                    break;
                }
                k--;
            }
            int gaps = 0;
            while (k >= 0) {
                if (school_class.at(day).at(k) == 0) {
                    gaps++;
                } 
                k--;
            }
            return gaps;
        };
        int gaps_before_update = 0;
        ensure_size(school_class, requirements.days, 0);
        for (int i = 0; i < requirements.days; i++) {
            gaps_before_update += calculate_gaps(school_class, i);
        }

        // Update data
        school_class.at(genome.day).at(genome.lesson)--;
        teacher.at(genome.day).at(genome.lesson)--;
        school_class.at(new_day_index).at(new_lesson_index)++;
        teacher.at(new_day_index).at(new_lesson_index)++;

        int gaps_after_update = 0;
        for (int i = 0; i < requirements.days; i++) {
            gaps_after_update += calculate_gaps(school_class, i);
        }
        int gaps = gaps_after_update - gaps_before_update;
        score += gaps * 10000;

        genome.day = new_day_index;
        genome.lesson = new_lesson_index;
    }

    Prototype() {}
    Prototype(const Requirements& requirements) {
        for (auto& school_class : requirements.classes) {
            for (auto& subject_load : school_class.second) {
                for (int i = 0; i < subject_load.second.min_weekly_load; i++) {
                    genomes.emplace_back(subject_load.first, subject_load.second.teacher, school_class.first);
                }
            }
        }
        score = 0;
        for (auto& genome : genomes) {
            if (class_data[genome.school_class].size() <= genome.day) {
                class_data.at(genome.school_class).resize(genome.day + 1);
            }
            if (class_data.at(genome.school_class).at(genome.day).size() <= genome.lesson) {
                class_data.at(genome.school_class).at(genome.day).resize(genome.lesson + 1, 0);
            }
            class_data.at(genome.school_class).at(genome.day).at(genome.lesson)++;

            if (teacher_data[genome.teacher].size() <= genome.day) {
                teacher_data.at(genome.teacher).resize(genome.day + 1);
            }
            if (teacher_data.at(genome.teacher).at(genome.day).size() <= genome.lesson) {
                teacher_data.at(genome.teacher).at(genome.day).resize(genome.lesson + 1, 0);
            }
            teacher_data.at(genome.teacher).at(genome.day).at(genome.lesson)++;
            if (teacher_data.at(genome.teacher).at(genome.day).at(genome.lesson) > 1) {
                score += 20000;
            }
            if (class_data.at(genome.school_class).at(genome.day).at(genome.lesson) > 1) {
                score += 20000;
            } 
        } 
        for (auto& school_class : class_data) {
            for (auto& day : school_class.second) {
                if (day.size() == 0) {
                    continue;
                }
                int i = day.size() - 1;
                while (i >= 0) {
                    if (day.at(i) != 0) {
                        break;
                    }
                    i--;
                }
                int gaps = 0;
                while (i >= 0) {
                    if (day.at(i) == 0) {
                        gaps++;
                    }
                    i--;
                }
                std::cout << gaps << std::endl;
                score += 10000 * gaps;
            }
        }
    } 
}; 

struct Schedule {
    struct Lesson {
        std::shared_ptr<Subject> subject;
        std::shared_ptr<Teacher> teacher;
    };

    std::map<std::shared_ptr<Class>, std::vector<std::vector<Lesson>>> class_schedule;
    std::map<std::shared_ptr<Teacher>, std::vector<std::vector<Lesson>>> teacher_schedule;

    Schedule() {}
    Schedule(const Schedule& other) : class_schedule(other.class_schedule), teacher_schedule(other.teacher_schedule) {}
    Schedule(const Prototype& prototype) {
        for (auto& genome : prototype.genomes) {
            if (!class_schedule.contains(genome.school_class)) {
                class_schedule.emplace(genome.school_class, std::vector<std::vector<Lesson>>(genome.day + 1));
            }
            auto& class_ref = class_schedule.at(genome.school_class);
            if (class_ref.size() <= genome.day) {
                class_ref.resize(genome.day + 1);
            }
            if (class_ref.at(genome.day).size() <= genome.lesson) {
                class_ref.at(genome.day).resize(genome.lesson + 1);
            }
            class_ref.at(genome.day).at(genome.lesson).teacher = genome.teacher;
            class_ref.at(genome.day).at(genome.lesson).subject = genome.subject;
        }
    }
    std::string string() const {
        std::string result;
        for (auto& school_class : class_schedule) {
            result += school_class.first->name + " {\n";
            for (int i = 0; i < school_class.second.size(); i++) {
                result += multiply_string(" ", 4) + "day " + std::to_string(i) + " {\n";
                for (int j = 0; j < school_class.second.at(i).size(); j++) {
                    auto& lesson = school_class.second.at(i).at(j);
                    if (lesson.subject == nullptr || lesson.teacher == nullptr) {
                        continue;
                    }
                    result += multiply_string(" ", 8) + std::to_string(j) + ". " + lesson.subject->name + " - " + lesson.teacher->name +"\n";
                }
                result += multiply_string(" ", 4) + "}\n";
            }
            result += " }\n";
        }
        return result;
        for (auto& teacher : teacher_schedule) {
            result += teacher.first->name + " {\n";
            for (int i = 0; i < teacher.second.size(); i++) {
                result += multiply_string(" ", 4) + "day " + std::to_string(i) + " {\n";
                for (int j = 0; j < teacher.second.at(i).size(); j++) {
                    auto& lesson = teacher.second.at(i).at(j);
                    result += multiply_string(" ", 8) + std::to_string(j) + ". " + lesson.subject->name + "\n";
                }
                result += multiply_string(" ", 4) + "}\n";
            }
            result += " }\n";
        }
        return result;
    }
};

Prototype build_schedule(const Requirements& requirements, int target = 1000) {
    std::vector<Prototype> prototypes = { Prototype(requirements) }; 
    int population = 50; 
    int elitism = 5; 
    for (int i = 0; i < 500000; i++) { 
        for (int j = 0; j < population; j++) { 
            if (prototypes.size() >= population) { 
                break; 
            } 
            prototypes.emplace_back(prototypes.at(j % elitism)); 
            prototypes.back().mutate(requirements); 
        } 
        std::sort(prototypes.begin(), prototypes.end(), [](const Prototype& a, const Prototype& b){ 
            return a.score < b.score; 
        }); 
        if (prototypes.front().score <= target) {
            return prototypes.front();
        }
        while (prototypes.size() > elitism) {
            prototypes.pop_back();
        }
        std::cout << "best=" << prototypes.front().score << std::endl;
    }
    return prototypes.front();
}

int main() {
    std::vector<std::shared_ptr<Subject>> subjects = {
    std::make_shared<Subject>("Math"),
    std::make_shared<Subject>("English Language Arts"),
    std::make_shared<Subject>("Science"),
    std::make_shared<Subject>("Social Studies"),
    std::make_shared<Subject>("Art"),
    std::make_shared<Subject>("Music"),
    std::make_shared<Subject>("Physical Education"),
    std::make_shared<Subject>("Computer Science"),
    std::make_shared<Subject>("Health")
    };

    std::vector<std::shared_ptr<Teacher>> teachers;
    for (int i = 0; i < 180; ++i) {
        teachers.push_back(std::make_shared<Teacher>("T" + std::to_string(i + 1), std::vector<Subject>()));
    }

    std::vector<std::shared_ptr<Class>> classes;
    for (int i = 0; i < 100; ++i) {
        std::string name = "C" + (i < 9 ? std::string("0") : std::string("")) + std::to_string(i + 1);
        classes.push_back(std::make_shared<Class>(name));
    }

    Requirements requirements;
    for (int i = 0; i < 20; ++i) {
        int group = i / 5; // 0..9, ten groups of five classes
        std::map<std::shared_ptr<Subject>, Requirements::SubjectRequirements> reqs;

        for (int j = 0; j < 9; ++j) {
            int teacherIndex = j * 10 + group; // 10 teachers per subject, one per group
            reqs[subjects.at(j)] = Requirements::SubjectRequirements(5, 1, teachers.at(teacherIndex));
        }

        requirements.classes[classes.at(i)] = reqs;
    }
    auto built = build_schedule(requirements);
    for (auto& genome : built.genomes) {
        std::cout << genome.string() << "\n";
    }
    std::cout << Schedule(built).string();
    return 0;
}
