#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include <optional>
#include <string_view>
#include <unordered_set>

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

