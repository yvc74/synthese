set_source_groups()

set(messages_SRCS
AddScenarioAction.cpp
AddScenarioAction.h
Alarm.cpp
Alarm.h
AlarmAddLinkAction.cpp
AlarmAddLinkAction.h
AlarmObjectLink.cpp
AlarmObjectLink.h
AlarmObjectLinkException.cpp
AlarmObjectLinkException.h
AlarmObjectLinkTableSync.cpp
AlarmObjectLinkTableSync.h
AlarmRecipient.cpp
AlarmRecipient.h
AlarmRecipientTemplate.h
AlarmRemoveLinkAction.cpp
AlarmRemoveLinkAction.h
AlarmTableSync.cpp
AlarmTableSync.h
BroadcastPoint.cpp
BroadcastPoint.hpp
BroadcastPointAlarmRecipient.cpp
BroadcastPointAlarmRecipient.hpp
ClearAllBroadcastCachesAction.cpp
ClearAllBroadcastCachesAction.hpp
CustomBroadcastPoint.cpp
CustomBroadcastPoint.hpp
CustomBroadcastPointTableSync.cpp
CustomBroadcastPointTableSync.hpp
CustomBroadcastPointsService.cpp
CustomBroadcastPointsService.hpp
FileNotificationChannel.hpp
FileNotificationChannel.cpp
GetMessagesFunction.cpp
GetMessagesFunction.hpp
HttpNotificationChannel.hpp
HttpNotificationChannel.cpp
MediaLibrary.cpp
MediaLibrary.hpp
MediaLibrariesService.cpp
MediaLibrariesService.hpp
MediaLibraryTableSync.cpp
MediaLibraryTableSync.hpp
MessageAdmin.cpp
MessageAdmin.h
MessageAlternative.cpp
MessageAlternative.hpp
MessageAlternativeTableSync.cpp
MessageAlternativeTableSync.hpp
MessageApplicationPeriod.hpp
MessageApplicationPeriod.cpp
MessageApplicationPeriodTableSync.cpp
MessageApplicationPeriodTableSync.hpp
MessagesLibraryLog.cpp
MessagesLibraryLog.h
MessagesLibraryRight.cpp
MessagesLibraryRight.h
MessagesLog.cpp
MessagesLog.h
MessagesModule.cpp
MessagesModule.gen.cpp
MessagesModuleRegister.cpp
MessagesModule.h
MessagesModule.inc.cpp
MessagesRight.cpp
MessagesRight.h
MessagesSection.cpp
MessagesSection.hpp
MessagesSectionTableSync.cpp
MessagesSectionTableSync.hpp
MessagesSectionsService.cpp
MessagesSectionsService.hpp
MessagesTypes.h
MessageTag.cpp
MessageTag.hpp
MessageTagsService.cpp
MessageTagsService.hpp
MessageTagTableSync.cpp
MessageTagTableSync.hpp
MessageType.cpp
MessageType.hpp
MessageTypesService.cpp
MessageTypesService.hpp
MessageTypeTableSync.cpp
MessageTypeTableSync.hpp
NewMessageAction.cpp
NewMessageAction.h
NotificationChannel.hpp
NotificationChannel.cpp
NotificationEvent.cpp
NotificationEvent.hpp
NotificationEventsService.cpp
NotificationEventsService.hpp
NotificationEventTableSync.cpp
NotificationEventTableSync.hpp
NotificationLog.cpp
NotificationLog.hpp
NotificationProvider.hpp
NotificationProvider.cpp
NotificationProvidersService.cpp
NotificationProvidersService.hpp
NotificationProviderTableSync.hpp
NotificationProviderTableSync.cpp
NotificationProviderTestService.cpp
NotificationProviderTestService.hpp
Scenario.cpp
Scenario.h
ScenarioAutopilot.hpp
ScenarioAutopilot.cpp
ScenarioCalendar.cpp
ScenarioCalendar.hpp
ScenarioCalendarTableSync.cpp
ScenarioCalendarTableSync.hpp
ScenarioDisplayFunction.cpp
ScenarioDisplayFunction.hpp
ScenarioFolder.cpp
ScenarioFolder.h
ScenarioFolderAdd.cpp
ScenarioFolderAdd.h
ScenarioFoldersService.cpp
ScenarioFoldersService.hpp
ScenarioFolderTableSync.cpp
ScenarioFolderTableSync.h
ScenarioFolderUpdateAction.cpp
ScenarioFolderUpdateAction.h
ScenarioSaveAction.cpp
ScenarioSaveAction.h
ScenariosListFunction.cpp
ScenariosListFunction.hpp
ScenarioRemoveAction.cpp
ScenarioRemoveAction.h
ScenarioStopAction.cpp
ScenarioStopAction.h
ScenarioTableSync.cpp
ScenarioTableSync.h
SentScenarioTableSync.cpp
SentScenarioTableSync.h
ScenarioTemplateTableSync.cpp
ScenarioTemplateTableSync.h
ScenarioTemplate.cpp
ScenarioTemplate.h
SentScenarioDao.hpp
SentScenario.cpp
SentScenario.h
SimpleMessageCreationAction.cpp
SimpleMessageCreationAction.hpp
SmtpNotificationChannel.hpp
SmtpNotificationChannel.cpp
UpdateAlarmMessagesAction.cpp
UpdateAlarmMessagesAction.h
UpdateAlarmMessagesFromTemplateAction.cpp
UpdateAlarmMessagesFromTemplateAction.h
)

set_source_files_properties(MessagesModule.gen.cpp MessagesModule.inc.cpp PROPERTIES HEADER_FILE_ONLY 1)

add_library(17_messages ${messages_SRCS})

target_link_libraries(17_messages
  12_security
  # Forward dependencies:
  #11_cms
)

install(TARGETS 17_messages DESTINATION lib)
