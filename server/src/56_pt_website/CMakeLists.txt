set_source_groups()

set(pt_website_SRCS
AddUserFavoriteJourneyAction.cpp
AddUserFavoriteJourneyAction.h
CityListFunction.cpp
CityListFunction.h
HourPeriod.cpp
HourPeriod.h
HourPeriodsField.cpp
HourPeriodsField.hpp
ObjectSiteLink.cpp
ObjectSiteLink.h
ObjectSiteLinkTableSync.cpp
ObjectSiteLinkTableSync.h
PlacesListFunction.cpp
PlacesListFunction.h
PlacesListService.cpp
PlacesListService.hpp
RollingStockFilter.cpp
RollingStockFilter.h
RollingStockFilterTableSync.cpp
RollingStockFilterTableSync.h
ServiceDetailService.cpp
ServiceDetailService.hpp
ServiceInformationsFunction.cpp
ServiceInformationsFunction.hpp
ServicesListService.cpp
ServicesListService.hpp
SiteCityAddAction.cpp
SiteCityAddAction.hpp
SiteCommercialLineLink.cpp
SiteCommercialLineLink.h
SiteUpdateAction.cpp
SiteUpdateAction.h
TransportSiteAdmin.cpp
TransportSiteAdmin.h
PTServiceConfig.cpp
PTServiceConfig.hpp
PTServiceConfigTableSync.cpp
PTServiceConfigTableSync.hpp
TransportWebsiteModule.cpp
TransportWebsiteModule.gen.cpp
TransportWebsiteModuleRegister.cpp
TransportWebsiteModule.h
TransportWebsiteModule.inc.cpp
TransportWebsiteRight.cpp
TransportWebsiteRight.h
TransportWebsiteTypes.hpp
UserBookmarksService.cpp
UserBookmarksService.hpp
UserFavoriteJourney.cpp
UserFavoriteJourney.h
UserFavoriteJourneyTableSync.cpp
UserFavoriteJourneyTableSync.h
)

set_source_files_properties(TransportWebsiteModule.gen.cpp TransportWebsiteModule.inc.cpp PROPERTIES HEADER_FILE_ONLY 1)

add_library(56_pt_website ${pt_website_SRCS})

target_link_libraries(56_pt_website
  11_cms
  53_pt_journey_planner
  # Forward dependencies:
  #59_road_journey_planner
)

install(TARGETS 56_pt_website DESTINATION lib)
