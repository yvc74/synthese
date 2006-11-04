
#ifndef SYNTHESE_ENV_MESSAGES_SCENARIO_ADMIN_H
#define SYNTHESE_ENV_MESSAGES_SCENARIO_ADMIN_H

#include "11_interfaces/AdminInterfaceElement.h"

namespace synthese
{
	namespace interfaces
	{

		/** Ecran d'�dition de sc�nario de diffusion de message.
			@ingroup m15

			@image html cap_admin_scenario.png
			@image latex cap_admin_scenario.png "Maquette de l'�cran d'�dition de sc�nario" width=14cm
			
			<i>Titre de la fen�tre</i> :
				- SYNTHESE Admin - Messages - Biblioth�que - Sc�narios - Interruption m�tro

			<i>Barre de navigation</i> :
				- Lien vers synthese::interfaces::AdminHome
				- Lien vers synthese::interfaces::MessagesAdmin
				- Lien vers synthese::interfaces::MessagesLibraryAdmin
				- Texte <tt>Sc�nario</tt>
				- Texte [Nom]

			<i>Zone de contenu</i> : <b>Formulaire d'�dition</b> :
				-# <b>Liste de messages du sc�nario</b>
					-# <tt>Sel</tt> : Permet la s�lection du message en vue d'une duplication
					-# <tt>Message</tt> : Texte rappelant le contenu du message. Un clic sur le texte se rend sur l'�cran d'�dition du message.
					-# <tt>Emplacement</tt> : Texte rappelant l'emplacement de diffusion au niveau logique. Un clic sur le texte se rend sur l'�cran d'�dition de l'emplacement.
					-# Bouton <tt>Modifier</tt> : Se rend vers l'�cran d'�dition du message s�lectionn�
					-# Bouton <tt>Supprimer</tt> : Supprime le message du sc�nario apr�s une demande de confirmation
				-# Le <b>bouton Ajouter</b> permet l'ajout d'un nouveau message au sc�nario :
					- si aucun message n'est s�lectionn� alors un message vide est ajout�
					- si un message existant est s�lectionn� alors son contenu est copi� dans le nouveau
			
			<i>S�curit�</i>
				- Une habilitation publique MessagesLibraryRight de niveau WRITE est n�cessaire pour acc�der � la page et y effectuer toutes les op�rations disponibles.
				
			<i>Journaux</i> : Les �v�nements suivants entrainent la cr�ation d'une entr�e dans le journal des messages MessagesLibraryLog :
				- INFO : Ajout de message au sc�nario
				- INFO : Suppression de message du sc�nario

		*/
		class MessagesScenarioAdmin : public AdminInterfaceElement
		{

		};
	}
}

#endif