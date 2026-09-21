// Событие в том виде, в каком его отдаёт журнал.
//
// Это агрегат, и пока намеренно только агрегат: ни методов, ни инвариантов.
// Классы начинаются с лекции 4, а до тех пор тип — просто набор полей,
// и код вокруг него обязан сам следить за тем, что в этих полях лежит.
// К занятию 2.1 у Event появятся приватные поля, обязательные ts и type
// в конструкторе и const-геттеры, и разница между «набором полей» и «типом
// с инвариантом» станет видна на своём же коде.
//
// Все поля — строки. Так их отдаёт журнал, и превращать их в числа здесь
// незачем: это дело слоя доступа из занятия 1.3, где заодно решается, что
// делать с полем, которого нет, и с полем, которое не разбирается.

#ifndef NANO_EDR_KIT_EVENT_H
#define NANO_EDR_KIT_EVENT_H

#include <string>
#include <vector>

namespace nano_edr {

struct Field {
    std::string key;
    std::string value;
};


// ts=1730000001000 type=process_start pid=1042 ||| ppid=880 image="C:\Windows\System32\cmd.exe" cmdline="cmd /c wscript %TEMP%\a.js" user=DESKTOP\max
struct Event {
    std::string ts;
    std::string type;
    std::string pid;

    std::vector<Field> fields;
};

}  // namespace nano_edr

#endif  // NANO_EDR_KIT_EVENT_H
