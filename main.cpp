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


struct Schedule {
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
    struct Gene {
        int lesson = 0;
        int day = 0;
        std::shared_ptr<Subject> subject;
        std::shared_ptr<Teacher> teacher;
        std::shared_ptr<Class> school_class;

        Gene() {}
        Gene(std::shared_ptr<Subject> subject, std::shared_ptr<Teacher> teacher, std::shared_ptr<Class> school_class) : subject(subject), teacher(teacher), school_class(school_class) {}
    };
    struct ScheduleViolations {
        int teacher_double_booking = 0;
        int class_double_booking = 0;
        int class_gaps = 0;
        int lesson_interval_exceeded = 0;
    };

    std::vector<Gene> genes;

    Schedule() {}
    Schedule(const Requirements& requirements) {
        for (auto& school_class : requirements.classes) {
            for (auto& subject_load : school_class.second) {
                for (int i = 0; i < subject_load.second.min_weekly_load; i++) {
                    genes.emplace_back(subject_load.first, subject_load.second.teacher, school_class.first);
                }
            }
        }
    }
    using SubjectMatrix = std::vector<std::vector<int>>;
    ScheduleViolations validate(const Requirements& requirements) {
        ScheduleViolations violations;
        std::map<std::shared_ptr<Class>, SubjectMatrix> class_data;
        std::map<std::shared_ptr<Teacher>, SubjectMatrix> teacher_data;
        for (auto& school_class : requirements.classes) {
            class_data.emplace(school_class.first, 
                SubjectMatrix(requirements.days)
            );
        }
        for (auto& gene : genes) {
            auto& school_class = class_data.at(gene.school_class);
            if (school_class.at(gene.day).size() <= gene.lesson) {
                school_class.at(gene.day).resize(gene.lesson + 1, 0);
            }
            if (school_class.at(gene.day).at(gene.lesson) != 0) {
                violations.class_double_booking++;
            }
            school_class.at(gene.day).at(gene.lesson)++;

            auto& teacher = teacher_data[gene.teacher];
            if (teacher.size() == 0) {
                teacher.resize(requirements.days);
            }
            if (teacher.at(gene.day).size() <= gene.lesson) {
                teacher.at(gene.day).resize(gene.lesson + 1, 0);
            }
            if (teacher.at(gene.day).at(gene.lesson) != 0) {
                violations.teacher_double_booking++;
            }
            teacher.at(gene.day).at(gene.lesson)++;
        }
        for (auto& school_class : class_data) {
            for (auto& day : school_class.second) {
                int lessons = 0;
                for (int lesson : day) {
                    if (lesson != 0) {
                        lessons++;
                    }
                }
                violations.class_gaps += day.size() - lessons;
                violations.lesson_interval_exceeded += std::pow(abs(lessons - 9), 3) * 10;
            }
        }
        return violations;
    }
    float analyze(const Requirements& requirements) {
        float rating = 0.0;
        std::map<std::shared_ptr<Class>, SubjectMatrix> class_data;
        std::map<std::shared_ptr<Teacher>, SubjectMatrix> teacher_data;
        for (auto& school_class : requirements.classes) {
            class_data.emplace(school_class.first, 
                SubjectMatrix(requirements.days)
            );
        }
        for (auto& gene : genes) {
            auto& school_class = class_data.at(gene.school_class);
            if (school_class.at(gene.day).size() <= gene.lesson) {
                school_class.at(gene.day).resize(gene.lesson + 1, 0);
            }
            if (school_class.at(gene.day).at(gene.lesson) != 0) {
                rating += 20000;
            }
            school_class.at(gene.day).at(gene.lesson)++;

            auto& teacher = teacher_data[gene.teacher];
            if (teacher.size() == 0) {
                teacher.resize(requirements.days);
            }
            if (teacher.at(gene.day).size() <= gene.lesson) {
                teacher.at(gene.day).resize(gene.lesson + 1, 0);
            }
            if (teacher.at(gene.day).at(gene.lesson) != 0) {
                rating += 20000;
            }
            teacher.at(gene.day).at(gene.lesson)++;
        }
        for (auto& school_class : class_data) {
            for (auto& day : school_class.second) {
                int lessons = 0;
                for (int lesson : day) {
                    if (lesson != 0) {
                        lessons++;
                    }
                }
                rating += std::pow(day.size() - lessons, 1) * 10000;
                rating += std::pow(abs(lessons - 9), 3) * 10;
            }
        }
        return rating;
    }
    void mutate(const Requirements& requirements) {
        auto& gene = get_random(genes);
        gene.lesson = randint(0, 10);
        gene.day = randint(0, requirements.days - 1);
    }
    std::string string() {
        std::string result;
        for (int i = 0; i < genes.size(); i++) {
            auto& gene = genes.at(i);
            result += "gene " + std::to_string(i) + ": " + "day = " + std::to_string(gene.day) + ", lesson = " + std::to_string(gene.lesson)
                + ", subject = " + gene.subject->name + ", teacher = " + gene.teacher->name + ", class = " + gene.school_class->name;
            if (i + 1 != genes.size()) {
                result += "\n";
            }
        }
        return result;
    }
};


struct ConstructedSchedule {
    struct Lesson {
        std::shared_ptr<Subject> subject;
        std::shared_ptr<Teacher> teacher;
    };

    std::map<std::shared_ptr<Class>, std::vector<std::vector<Lesson>>> class_schedule;
    std::map<std::shared_ptr<Teacher>, std::vector<std::vector<Lesson>>> teacher_schedule;

    ConstructedSchedule() {}
    ConstructedSchedule(const ConstructedSchedule& other) : class_schedule(other.class_schedule), teacher_schedule(other.teacher_schedule) {}
    ConstructedSchedule(const Schedule& schedule) {
        for (auto& gene : schedule.genes) {
            auto& class_ref = class_schedule[gene.school_class];
            if (class_ref.size() <= gene.day) {
                class_ref.resize(gene.day + 1);
            }
            if (class_ref.at(gene.day).size() <= gene.lesson) {
                class_ref.at(gene.day).resize(gene.lesson + 1);
            }
            class_ref.at(gene.day).at(gene.lesson).teacher = gene.teacher;
            class_ref.at(gene.day).at(gene.lesson).subject = gene.subject;
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
                    result += multiply_string(" ", 8) + std::to_string(j) + ". " + lesson.subject->name + " - " + lesson.teacher->name +"\n";
                }
                result += multiply_string(" ", 4) + "}\n";
            }
            result += " }\n";
        }
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

Schedule create_schedule(const Requirements& requirements) {
    Schedule schedule(requirements);
    int population = 50;
    int survivors = 5;
    std::vector<Schedule> models;
    for (int i = 0; i < population; i++) {
        models.emplace_back(schedule);
        models.back().mutate(requirements);
    }

    for (int i = 0; i < 5000; i++) {
        std::vector<std::pair<float, Schedule>> analysis;
        for (auto& model : models) {
            int analysis_result = model.analyze(requirements);
            if (analysis_result == 0) {
                return model;
            }
            analysis.emplace_back(analysis_result, std::move(model));
        }
        std::sort(analysis.begin(), analysis.end(), [](const std::pair<float, Schedule>& a, const std::pair<float, Schedule>& b){
            return a.first < b.first;
        });
        std::cout << "best = " << analysis.front().first << std::endl;
        models.clear();
        for (int i = 0; i < survivors; i++) {
            models.emplace_back(std::move(analysis.at(i).second));
        }
        assert(models.size() != 0);
        for (int k = 0; k < population; k++) {
            models.emplace_back(models.at(k % survivors));
            models.back().mutate(requirements);
        }
    }
    return models.front();
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
for (int i = 0; i < 90; ++i) {
    teachers.push_back(std::make_shared<Teacher>("T" + std::to_string(i + 1), std::vector<Subject>()));
}

std::vector<std::shared_ptr<Class>> classes;
for (int i = 0; i < 50; ++i) {
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
    auto built = create_schedule(requirements);
    std::cout << built.string() << std::endl;
    std::cout << built.analyze(requirements) << std::endl;
    ConstructedSchedule constructed_schedule(built);
    std::cout << constructed_schedule.string();
    return 0;
}
