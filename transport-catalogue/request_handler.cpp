#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */


 /*
 using namespace std;

 namespace TransportCatalogue {
	 // MapRenderer понадобится в следующей части итогового проекта
 RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
		 : db_(db)
		 ,renderer_(renderer){}

		 // Возвращает информацию о маршруте (запрос Bus)
 optional<BusInfo> RequestHandler::GetBusStat(const string_view& bus_name) const {
	 if (!db_.FindBus(bus_name)) {
		 return db_.GetBusInfo(bus_name);
	 }
	 return nullopt;
	 }

	 // Возвращает маршруты, проходящие через
 //const std::unordered_set<Bus*>* RequestHandler::GetBusesByStop(const string_view& stop_name) const {

 //	}

	 // Этот метод будет нужен в следующей части итогового проекта
 //	svg::Document RenderMap() const;
 }  //namespace TransportCatalogue
 */