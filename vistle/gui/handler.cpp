/*********************************************************************************/
/*! \file handler.cpp
 *
 * Contains three classes:
 * 1. Handler -- currently empty, is intended to be used to handle any instructions
 *    sent to the UiRunner.
 * 2. UiRunner -- simple class, handling commands dispatched to vistle's userInterface.
 * 3. StatePrinter -- observer class that watches for changes in vistle, and sends
 *    signals to the MainWindow.
 */
/**********************************************************************************/
#include "handler.h"

#include <QString>

namespace gui {

/*************************************************************************/
// begin class VHandler
/*************************************************************************/
Handler::Handler()
{
}

UiRunner *UiRunner::s_instance = nullptr;

/*************************************************************************/
// begin class UiRunner
/*************************************************************************/

UiRunner::UiRunner(vistle::UserInterface &ui) : m_ui(ui)
{
   assert(s_instance == nullptr);
   s_instance = this;
}

UiRunner &UiRunner::the() {

   assert(s_instance);
   return *s_instance;
}

void UiRunner::cancel() {
    boost::unique_lock<boost::mutex> lock(m_mutex);
	m_done = true;
}

void UiRunner::operator()() {
	while(m_ui.dispatch()) {
        boost::unique_lock<boost::mutex> lock(m_mutex);
		if (m_done) {
			break;
		}
		usleep(10000);
	}
}

vistle::UserInterface &UiRunner::ui() const {

   return m_ui;
}

void UiRunner::sendMessage(const vistle::message::Message &msg) const
{
   ui().sendMessage(msg);
}

vistle::Parameter *UiRunner::getParameter(int id, QString name) const
{
  vistle::Parameter *p = ui().state().getParameter(id, name.toStdString());
  if (!p) {
     std::cerr << "no such parameter: " << id << ":" << name.toStdString() << std::endl;
  }
  return p;
}

/*************************************************************************/
// begin class StatePrinter
/*************************************************************************/

/*!
 * \brief StatePrinter::StatePrinter
 * \param parent
 *
 */
VistleObserver::VistleObserver(QObject *parent) : QObject(parent)
{

}

void VistleObserver::newModule(int moduleId, const boost::uuids::uuid &spawnUuid, const std::string &moduleName) {
	QString name = QString::fromStdString(moduleName);
   emit newModule_s(moduleId, spawnUuid, name);
}

void VistleObserver::deleteModule(int moduleId) {
	emit deleteModule_s(moduleId);
}

void VistleObserver::moduleStateChanged(int moduleId, int stateBits) {
    if (stateBits & StateObserver::Initialized) emit moduleStateChanged_s(moduleId, stateBits, INITIALIZED);
    if (stateBits & StateObserver::Killed) emit moduleStateChanged_s(moduleId, stateBits, KILLED);;
    if (stateBits & StateObserver::Busy) emit moduleStateChanged_s(moduleId, stateBits, BUSY);;
}

void VistleObserver::newParameter(int moduleId, const std::string &parameterName) {
	QString name = QString::fromStdString(parameterName);
	emit newParameter_s(moduleId, name);
}

void VistleObserver::parameterValueChanged(int moduleId, const std::string &parameterName) {
	QString name = QString::fromStdString(parameterName);
	emit parameterValueChanged_s(moduleId, name);
}

void VistleObserver::newPort(int moduleId, const std::string &portName) {
	QString name = QString::fromStdString(portName);
	emit newPort_s(moduleId, name);
}

void VistleObserver::newConnection(int fromId, const std::string &fromName,
                   				 int toId, const std::string &toName) {
    QString name = QString::fromStdString(fromName);
    QString name2 = QString::fromStdString(toName);
    emit newConnection_s(fromId, name, toId, name2);
}

void VistleObserver::deleteConnection(int fromId, const std::string &fromName,
                      				int toId, const std::string &toName) {
    QString name = QString::fromStdString(fromName);
    QString name2 = QString::fromStdString(toName);
    emit deleteConnection_s(fromId, name, toId, name2);
}

} //namespace gui
