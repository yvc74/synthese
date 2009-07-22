
/** AdminInterfaceElement class header.
	@file AdminInterfaceElement.h

	This file belongs to the SYNTHESE project (public transportation specialized software)
	Copyright (C) 2002 Hugues Romain - RCS <contact@reseaux-conseil.com>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef SYNTHESE_INTERFACES_ADMIN_INTERFACE_ELEMENT_H
#define SYNTHESE_INTERFACES_ADMIN_INTERFACE_ELEMENT_H

#include "FactoryBase.h"
#include "11_interfaces/Types.h"
#include "FunctionRequest.h"

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/logic/tribool.hpp>

namespace synthese
{
	namespace security
	{
		class Right;
	}
	
	namespace server
	{
		class ParametersMap;
	}

	namespace admin
	{
	    class AdminRequest;

		////////////////////////////////////////////////////////////////////
		/// Composant d'administration.
		///	@ingroup m14
		///	
		///	Un composant d'administration est un formulaire param�trable, destin� � effectuer une �criture en temps r�el dans la base SYNTHESE, dans un but de param�trage de l'application (il ne s'agit pas du seul moyen d'�criture).
		///	Une m�me donn�e peut �tre mise � jour par autant de composants que n�cessaire, d�fini avant tout sur la base d'une ergonomie de qualit�.
		///	Chaque composant est rattach� au module correspondant aux donn�es � modifier sous forme d'une sous-classe de AdminInterfaceElement. 
		///	Le comportement des composants d'administration est en g�n�ral d�fini d'apr�s les @ref defRight "habilitations" de l'utilisateur connect�.
		///	Par exemple :
		///		- un utilisateur anonyme n'a acc�s � aucun composant d'administration
		///		- un administrateur a acc�s � tous les composants d'administration.
		////////////////////////////////////////////////////////////////////
		class AdminInterfaceElement:
			public util::FactoryBase<AdminInterfaceElement>,
			public boost::enable_shared_from_this<AdminInterfaceElement>
		{
		public:
			typedef std::vector<boost::shared_ptr<const AdminInterfaceElement> > PageLinks;
			
			template<class T>
			static void AddToLinks(PageLinks& links, boost::shared_ptr<T> o)
			{
				links.push_back(boost::static_pointer_cast<const AdminInterfaceElement, T>(o));
			}
			
			////////////////////////////////////////////////////////////////////
			/// Tree of links to administrative pages.
			////////////////////////////////////////////////////////////////////
			struct PageLinksTree
			{
				boost::shared_ptr<const AdminInterfaceElement>	page;
				std::vector<PageLinksTree>		subPages;
				bool							isNodeOpened;

				PageLinksTree(
					boost::shared_ptr<const AdminInterfaceElement> page_
				):	page(page_), subPages(), isNodeOpened(false) {}
				PageLinksTree(
				):	page(), subPages(), isNodeOpened(false) {}
			};



			////////////////////////////////////////////////////////////////////
			/// Tab and its corresponding div in content of administrative page.
			///
			///	To use tabs :
			///		- call begin method to start displaying the div corresponding
			///			to the first tab. Test the output of the method to decide if 
			///			the content can be displayed. 
			///		- call begin for each following tab (the ending of the
			///			preceding one is automatic
			///		- call end to close the last div.
			///	
			///	The tabs list is displayed by the library interface element
			///	AdminTabsInterfaceElement
			///
			/// The generated HTML code is a div element by tab containing its
			/// corresponding content. The class of the div is @c tabdiv.
			/// The id of the div is @c tab_(id)_content.
			////////////////////////////////////////////////////////////////////
			class Tab
			{
			private:
				std::string	_title;
				std::string	_id;
				std::string _icon;
				bool		_writePermission;



			public:
				////////////////////////////////////////////////////////////////////
				///	Tab constructor.
				///	@param title Title of the tab
				/// @param id ID of the tab
				/// @param writePermission the user has write permissions on the 
				///		content of the tab
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				Tab(
					std::string title = std::string(),
					std::string id = std::string(),
					bool writePermission = true,
					std::string icon = std::string()
				);
				
				
				////////////////////////////////////////////////////////////////////
				///	Title getter.
				///	@return Title of the tab.
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				const std::string& getTitle() const;

				
				
				////////////////////////////////////////////////////////////////////
				///	ID getter.
				///	@return ID of the tab.
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				const std::string& getId() const;

				
				
				////////////////////////////////////////////////////////////////////
				///	Write permission getter.
				///	@return bool true if the user can use the content of the tab
				///		with write permissions
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				bool getWritePermission() const;


				
				////////////////////////////////////////////////////////////////////
				///	Icon getter.
				///	@return Icon URL
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				std::string getIcon() const;
			};

			////////////////////////////////////////////////////////////////////
			/// Tab vector type.
			////////////////////////////////////////////////////////////////////
			typedef std::vector<Tab>	Tabs;


		private:
			mutable const Tab*	_currentTab;

		protected:

			//! \name Cache
			//@{
				mutable PageLinks		_treePosition;
				mutable PageLinksTree	_tree;
				mutable	Tabs			_tabs;
				mutable bool			_tabBuilded;
			//@}

			//! \name Properties
			//@{
				boost::shared_ptr<util::Env>	_env;
				std::string _activeTab;
			//@}

			PageLinksTree	_buildTreeRecursion(
				boost::shared_ptr<const AdminInterfaceElement> page,
				const PageLinks position,
				const server::FunctionRequest<admin::AdminRequest>& request
			) const;

			util::Env& _getEnv() const;

			
			
			////////////////////////////////////////////////////////////////////
			///	Tree generator.
			///	@author Hugues Romain
			///	@date 2008
			/// Generates the whole tree depending on the context.
			/// This method has to be called by the current displayed admin page.
			////////////////////////////////////////////////////////////////////
			void _buildTree(
				const server::FunctionRequest<admin::AdminRequest>& request
			) const;
			
			
public:
			//! \name Tabs management
			//@{
				////////////////////////////////////////////////////////////////////
				///	Tabs generator.
				///	@author Hugues Romain
				///	@date 2008
				/// Virtual method :
				///		- default behavior : creates nothing
				///		- can be overloaded : tabs creation depending on the context
				/// The _buildTabs method is in charge of : 
				///		- the control of the profile of the user to determine the 
				///			tabs list. 
				///		- to set _tabBuilded at true to avoid the method to be
				///			relaunched
				////////////////////////////////////////////////////////////////////
				virtual void _buildTabs(
					const server::FunctionRequest<admin::AdminRequest>& _request
				) const;



				////////////////////////////////////////////////////////////////////
				///	Displays a begin of a content linked by a tab.
				/// @param stream Stream to display the code on.
				///	@param id ID of the tab
				/// @return true if the content must be displayed
				///	@author Hugues Romain
				///	@date 2008
				/// The code is displayed only if the tab is present in the tabs
				/// list of the page (depending of the rights of the user)
				/// 
				/// If necessary, this method closes automatically the last displayed
				/// tab.
				////////////////////////////////////////////////////////////////////
				bool openTabContent(
					std::ostream& stream,
					const std::string& id
				) const;

				
				
				////////////////////////////////////////////////////////////////////
				///	Displays the end of a content linked by a tab.
				/// @param stream Stream to display the code on.
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				void closeTabContent(
					std::ostream& stream
				) const;

				
				
				////////////////////////////////////////////////////////////////////
				///	Test if the user has write permissions on the currently
				///	displayed tab.
				///	@return bool True if the user has write permissions
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				bool tabHasWritePermissions() const;



				////////////////////////////////////////////////////////////////////
				///	Generator of page internal link button to the tab content.
				/// @param tab ID of the tab to link
				///	@return the Link button HTML code if the tab exists, else nothing
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				std::string getTabLinkButton(
					const std::string& tab
				) const;
			//@}
				

			////////////////////////////////////////////////////////////////////
			///	AdminInterfaceElement constructor.
			///	@author Hugues Romain
			///	@date 2008
			////////////////////////////////////////////////////////////////////
			AdminInterfaceElement();

			//! \name Setters
			//@{
				////////////////////////////////////////////////////////////////////
				///	Active tab setter.
				///	@param value ID of the active tab at the page load. 
				///		Empty = first tab.
				///	@author Hugues Romain
				///	@date 2008
				////////////////////////////////////////////////////////////////////
				void setActiveTab(const std::string& value);
				
				void setEnv(
					boost::shared_ptr<util::Env> value
				);
			//@}


			//! \name Getters
			//@{
				const PageLinks&		getTreePosition(
					const server::FunctionRequest<admin::AdminRequest>& request
				)	const;
				const PageLinksTree&	getTree(
					const server::FunctionRequest<admin::AdminRequest>& request
				)	const;
				const Tabs&				getTabs()			const;
				std::string				getCurrentTab()		const;
				const std::string&		getActiveTab()		const;
			//@}


			//! \name Queries
			//@{
				////////////////////////////////////////////////////////////////////
				///	Tab getter.
				///	@param key key of the tab
				///	@return const Tab& the found tab
				///	@author Hugues Romain
				///	@date 2008
				/// @throw Exception if tab was not found
				////////////////////////////////////////////////////////////////////
				const Tab&	getTab(const std::string& key) const;
			//@}
			
			
			////////////////////////////////////////////////////////////////////
			///	displayTabs.
			///	@param stream
			///	@param variables
			///	@param request
			///	@return void
			///	@author Hugues Romain
			///	@date 2008
			////////////////////////////////////////////////////////////////////
			void displayTabs(
				std::ostream& stream,
				interfaces::VariablesMap& variables,
				const server::FunctionRequest<admin::AdminRequest>& request
			) const;



			//! \name Virtual initialization method
			//@{
				/// Initialization of the parameters from a request.
				///	@param request The request to use for the initialization.
				///
				/// Note to AdminInterfaceElement subclasses developers : the setFromParamtersMap method is intended to
				/// load all parameters from the map, in two ways :
				///  - storage of the parameters (directly or not)
				///  - running of several actions to prepare the display (such searching results of a SQL request)
				virtual void setFromParametersMap(
					const server::ParametersMap& map,
					bool objectWillBeCreatedLater
				) = 0;
			//@}

			//! \name Virtual output methods
			//@{
				/** Parameters map generator, used when building an url to the admin page.
					@return server::ParametersMap The generated parameters map
					@author Hugues Romain
					@date 2007					
				*/
				virtual server::ParametersMap getParametersMap() const = 0;

				/** Authorization control.
					@param request The current request
					@return bool True if the displayed page can be displayed
					@author Hugues Romain
					@date 2007					
				*/
				virtual bool isAuthorized(
					const server::FunctionRequest<admin::AdminRequest>& _request
				) const = 0;

				/** Display of the content of the admin element.
					@param stream Stream to write on.
					@param variables Execution variables
					@param request Current request
				*/
				virtual void display(
					std::ostream& stream,
					interfaces::VariablesMap& variables,
					const server::FunctionRequest<admin::AdminRequest>& _request
				) const = 0;

				virtual std::string getIcon() const = 0;
				virtual std::string getTitle() const = 0;
				

				/** Sub pages getter.
					@param request User request
					@return PageLinks Ordered vector of sub pages links
					@author Hugues Romain
					@date 2008
					
					The default implementation handles the auto registration of administrative components by getSuperiorVirtual() method.
					This method can be overloaded to create customized sub tree.
				*/
				virtual PageLinks getSubPages(
					boost::shared_ptr<const AdminInterfaceElement> currentPage,
					const server::FunctionRequest<admin::AdminRequest>& _request
				) const;



				virtual PageLinks getSubPagesOfModule(
					const std::string& moduleKey,
					boost::shared_ptr<const AdminInterfaceElement> currentPage,
					const server::FunctionRequest<admin::AdminRequest>& _request
				) const;


				boost::shared_ptr<AdminInterfaceElement> getNewPage() const;


				template<class T>
				boost::shared_ptr<T> getNewOtherPage(
					boost::logic::tribool copyParameters = boost::logic::indeterminate
				) const {
					boost::shared_ptr<T> p(new T);
					p->setEnv(_env);
					if(	copyParameters == true ||
						boost::logic::indeterminate(copyParameters) &&
						this->getFactoryKey() == T::FACTORY_KEY
					){
						p->setActiveTab(getCurrentTab());
						p->setFromParametersMap(
							getParametersMap(),
							false
						);
					}
					return p;
				}
				

				/** Gets the opening position of the node in the tree view.
					@return bool true = the page is visible, all the superior nodes are open, false = the page must not be visible, and will be hidden if no one another page of the same level must be visible.
					@author Hugues Romain
					@date 2008					
				*/
				virtual bool isPageVisibleInTree(
					const AdminInterfaceElement& currentPage,
					const server::FunctionRequest<admin::AdminRequest>& _request
				) const;
			//@}
		};
	}
}

#endif

