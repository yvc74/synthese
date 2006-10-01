
#ifndef SYNTHESE_SECURITY_USERS_ADMIN_H
#define SYNTHESE_SECURITY_USERS_ADMIN_H

namespace synthese
{
	namespace interfaces
	{
		/** Ecran de recherche et liste d'utilisateurs.
			@ingroup m05
		
			@image html cap_admin_users.png
			@image latex cap_admin_users.png "Maquette de l'�cran de recherche d'utilisateur" width=14cm

			<i>Barre de navigation</i> :
				- Lien vers synthese::interfaces::HomeAdmin
				- Texte <tt>Utilisateurs</tt>

			<i>Zone de contenus</i> :
				-# <b>Formulaire de recherche</b> :
					-# <tt>Login</tt> : champ texte : recherche des utilisateurs dont le login contient le texte saisi. Absence de texte : filtre d�sactiv�
					-# <tt>Nom</tt> : champ texte : recherche des utilisateurs dont le nom ou le pr�nom contient le texte saisi. Absence de texte : filtre d�sactiv�
					-# <tt>Profil</tt> : liste d�roulante contenant tous les profils correspondant � au moins un utilisateur existant :
						- si un profil est s�lectionn�, alors seuls s'affichent les utilisateurs ayant le profil s�lectionn�
						- si aucun profil n'est s�lectionn�, alors le filtre est d�sactiv�
					-# Bouton <tt>Rechercher</tt> : lancement de la recherche
				-# <b>Tableau des utilisateurs</b> : Les utilisateurs recherch�s sont d�crits par les colonnes suivantes :
					-# <tt>Sel</tt> : case � cocher permettant une s�lection d'un ou plusieurs utilisateurs en vue d'une suppression ou d'une copie
					-# <tt>Login</tt> : login de l'utilisateur
					-# <tt>Nom</tt> : pr�nom et nom de l'utilisateur
					-# <tt>Profil</tt> : profil de l'utilisateur
					-# Bouton <tt>Editer</tt> : conduit vers la page synthese::interfaces::UserAdmin pour l'utilisateur affich� sur la ligne
				-# <b>Ligne d'ajout d'utilisateur</b> :
					-# Champ <tt>Login</tt> : champ texte. Entrer ici le login de l'utilisateur. Celui-ci doit �tre unique. En cas de tentative de cr�ation d'utilisateur avec un login d�j� pris, la cr�ation est abandonn�e et un message d'erreur apparait :
						@code L'utilisateur ne peut �tre cr�� car le login entr� est d�j� pris. Veuillez choisir un autre login @endcode
					-# Champ <tt>Nom</tt> : champ texte. Entrer ici le nom de famille de l'utilisateur. Ce champ est obligatoire. S'il n'est pas rempli la cr�ation est abandonn�e et un message d'erreur apparait :
						@code L'utilisateur ne peut �tre cr�� car le nom n'est pas renseign�. Veuillez renseigner le champ nom. @endcode
                    -# Champ <tt>Profil</tt> : liste de choix. S�lectionner ici un profil d'habilitations. Champ obligatoire.
					-# Bouton <tt>Ajouter</tt> : cr�e l'utilisateur en fonction des valeur saisies. La saisie des informations personnelles de l'utilisateur se poursuit sur l'�cran synthese::interfaces::UserAdmin.
				-# Un maximum de 50 entr�es est affich� � l'�cran. En cas de d�passement de ce nombre d'apr�s les crit�res de recherche, un lien <tt>Entr�es suivantes</tt> apparait et permet de visualiser les entr�es suivantes. A partir de la seconde page, un lien <tt>Entr�es pr�c�dentes</tt> apparait �galement.
				-# Un <b>Bouton de suppression</b> permet de supprimer les utilisateurs s�lectionn�s gr�ce aux cases � cocher. Apr�s confirmation par une boite de dialogue, la suppression est effectu�e pour chaque utilisateur :
					- l'utilisateur est r�ellement supprim� si il n'est � l'origine d'aucune entr�e de journal
					- l'utilisateur est d�sactiv� si il est � l'origine d'au moins une entr�e de journal, afin de permettre d'acc�der � ses informations dans le cadre de la consultation ult�rieure du journal. En ce cas, son login est tout de m�me remis � disposition.
			
			<i>S�curit�</i>
				- L'affichage de la fen�tre en consultation n�cessite une habilitation securit� de niveau lecture
				- L'affichage de la fen�tre en mode �dition, suppression d'habilitations comprises, requiert une habilitation s�curit� de niveau �criture

			<i>Journaux</i> : Les op�rations suivantes sont consign�es dans le journal de s�curit� :
				- Cr�ation d'utilisateur
				- Suppression d'utilisateur : le choix "suppression ou d�sactivation" est notifi� dans l'entr�e
		*/
		class UsersAdmin: public AdminInterfaceElement
		{
		};
	}
}