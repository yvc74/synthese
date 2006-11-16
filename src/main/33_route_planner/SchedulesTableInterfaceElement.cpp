
#include "SchedulesTableInterfaceElement.h"
#include "11_interfaces/Site.h"
#include "11_interfaces/Interface.h"
#include "RoutePlannerNoSolutionInterfacePage.h"
#include "RoutePlannerSheetColumnInterfacePage.h"
#include "RoutePlannerSheetLineInterfacePage.h"
#include "04_time/DateTime.h"
#include "JourneyLeg.h"
#include "15_env/Service.h"
#include "15_env/Road.h"
#include "15_env/Edge.h"
#include "15_env/Vertex.h"
#include <vector>
#include <sstream>

namespace synthese
{
	using namespace routeplanner;
	using namespace time;
	using namespace env;
	using std::vector;
	using std::ostringstream;
	
	namespace interfaces
	{
		const bool SchedulesTableInterfaceElement::_registered = Factory<LibraryInterfaceElement>::integrate<SchedulesTableInterfaceElement>("schedules_table");


		void SchedulesTableInterfaceElement::display( std::ostream& stream, const ParametersVector& parameters, const void* object /*= NULL*/, const Site* site /*= NULL*/ ) const
		{
			const JourneyVector* jv = (const JourneyVector*) object;

			if ( jv == NULL )  // No solution or type error
			{
				const RoutePlannerNoSolutionInterfacePage* noSolutionPage = site->getInterface()->getPage<RoutePlannerNoSolutionInterfacePage>();
				noSolutionPage->display(stream, site);
			}
			else
			{
				const JourneyLeg* curET;
				size_t __Ligne;
				const PlaceList placesList = getStopsListForScheduleTable( *jv );
				DateTime lastDepartureDateTime;
				DateTime lastArrivalDateTime;
				Hour unknownTime( TIME_UNKNOWN );
				const RoutePlannerSheetColumnInterfacePage* columnInterfacePage = site->getInterface()->getPage<RoutePlannerSheetColumnInterfacePage>();
				const RoutePlannerSheetLineInterfacePage* lineInterfacePage = site->getInterface()->getPage<RoutePlannerSheetLineInterfacePage>();

				// Initialization of text lines
				vector<ostringstream*> __Tampons(placesList.size());
				for (vector<ostringstream*>::iterator it = __Tampons.begin(); it != __Tampons.end(); ++it)
					*it = new ostringstream;

				// Loop on each journey
				int i=1;
				for ( JourneyVector::const_iterator it = jv->begin(); it != jv->end(); ++it, ++i )
				{
					// Loop on each leg
					__Ligne=0;
					for (int l=0; l< it->getJourneyLegCount(); ++l)
					{
						curET = it->getJourneyLeg (l);
						
						// Saving of the columns on each lines
						columnInterfacePage->display( *__Tampons[__Ligne]
							, __Ligne == 0, true, i, dynamic_cast<const Road*> (curET->getService ()->getPath ()) != NULL
							, curET->getDepartureTime().getHour(), lastDepartureDateTime.getHour(), it->getContinuousServiceRange() > 0
							, site );
						
						for ( __Ligne++; placesList[ __Ligne ] != curET->getDestination()->getFromVertex ()->getConnectionPlace(); __Ligne++ )
							columnInterfacePage->display( *__Tampons[ __Ligne ]
								, true, true, i, false, unknownTime, unknownTime, false
								, site );
						
						columnInterfacePage->display( *__Tampons[ __Ligne ] 
							, true, l == it->getJourneyLegCount ()-1, i, dynamic_cast<const Road*> (curET->getService ()->getPath ()) != NULL
							, curET->getArrivalTime ().getHour (), lastArrivalDateTime.getHour(), it->getContinuousServiceRange() > 0
							, site );
					}
				}

				// Initialization of text lines
				bool __Couleur = false;
				for ( __Ligne = 0; __Ligne < placesList.size(); __Ligne++ )
				{
					lineInterfacePage->display( stream, __Tampons[ __Ligne ]->str(), __Couleur, placesList[ __Ligne ], site );
				}

				// Cleaning the string vector
				for (vector<ostringstream*>::iterator it = __Tampons.begin(); it != __Tampons.end(); ++it)
					delete *it;


				/*
				// GESTION DES ALERTES
				// Gestion des alertes : 3 cas possibles :
				// Alerte sur arr�t de d�part
				// Circulation � r�servation
				// Alerte sur circulation
				// Alerte sur arr�t d'arriv�e
				synthese::time::DateTime __debutAlerte, __finAlerte;

				// Alerte sur arr�t de d�part
				// D�but alerte = premier d�part
				// Fin alerte = dernier d�part
				synthese::time::DateTime debutPrem = curET->getDepartureTime();
				synthese::time::DateTime finPrem = debutPrem;
				if (__Trajet->getContinuousServiceRange ().Valeur())
				finPrem += __Trajet->getContinuousServiceRange ();
				if (curET->getGareDepart()->getAlarm().showMessage(__debutAlerte, __finAlerte)
				&& __NiveauRenvoiColonne < curET->getGareDepart()->getAlarm().Niveau())
				__NiveauRenvoiColonne = curET->getGareDepart()->getAlarm().Niveau();

				// Circulation � r�servation obligatoire
				synthese::time::DateTime maintenant;
				maintenant.setMoment();
				if (curET->getLigne()->GetResa()->TypeResa() == Obligatoire
				&& curET->getLigne()->GetResa()->reservationPossible(curET->getLigne()->GetTrain(curET->getService()), maintenant, curET->getDepartureTime())
				&& __NiveauRenvoiColonne < ALERTE_ATTENTION)
				__NiveauRenvoiColonne = ALERTE_ATTENTION;

				// Circulation � r�servation possible
				maintenant.setMoment();
				if (curET->getLigne()->GetResa()->TypeResa() == Facultative
				&& curET->getLigne()->GetResa()->reservationPossible(curET->getLigne()->GetTrain(curET->getService()), maintenant, curET->getDepartureTime())
				&& __NiveauRenvoiColonne < ALERTE_INFO)
				__NiveauRenvoiColonne = ALERTE_INFO;

				// Alerte sur circulation
				// D�but alerte = premier d�part
				// Fin alerte = derni�re arriv�e
				debutPrem = curET->getDepartureTime();
				finPrem = curET->getArrivalTime ();
				if (__Trajet->getContinuousServiceRange ().Valeur())
				finPrem += __Trajet->getContinuousServiceRange ();
				if (curET->getLigne()->getAlarm().showMessage(__debutAlerte, __finAlerte)
				&& __NiveauRenvoiColonne < curET->getLigne()->getAlarm().Niveau())
				__NiveauRenvoiColonne = curET->getLigne()->getAlarm().Niveau();

				// Alerte sur arr�t d'arriv�e
				// D�but alerte = premi�re arriv�e
				// Fin alerte = dernier d�part de l'arr�t si correspondnce, derni�re arriv�e sinon
				__debutAlerte = curET->getArrivalTime ();
				__finAlerte = __debutAlerte;
				if (curET->Suivant() != NULL)
				__finAlerte = curET->Suivant()->getDepartureTime();
				if (__Trajet->getContinuousServiceRange ().Valeur())
				__finAlerte += __Trajet->getContinuousServiceRange ();
				if (curET->getGareArrivee()->getAlarm().showMessage(__debutAlerte, __finAlerte)
				&& __NiveauRenvoiColonne < curET->getGareArrivee()->getAlarm().Niveau())
				__NiveauRenvoiColonne = curET->getGareArrivee()->getAlarm().Niveau();
				}

				// Affichage du renvoi si necessaire
				//    if (__NiveauRenvoiColonne)
				//    {
				//     TamponRenvois << "<img src=\"" << __RepBI << "img/warning.gif\" alt=\"Cliquez sur la colonne pour plus d'informations\" />";
				//     __MontrerLigneRenvois = true;
				//    }

				for (l++;l!=m;l++)
				{
				Tampons[l] << "<td class=\"tdHoraires";
				if (Couleur1)
				Tampons[l] << "1";
				else
				Tampons[l] << "2";
				Tampons[l] << "\" onclick=\"show('divFiche";
				Tampons[l].Copie(n,3);
				Tampons[l] << "')\">";
				if (Couleur1)
				Couleur1=false;
				else
				Couleur1=true;
				}

				//    pCtxt << "</tr></table></div>";
				//    TamponRenvois << "</td>";
				}
				*/

				//    delete [] Tampons;
			}

		}

		size_t SchedulesTableInterfaceElement::OrdrePAEchangeSiPossible( const JourneyVector& jv, PlaceList& pl, const LockedLinesList& lll, size_t PositionActuelle, size_t PositionGareSouhaitee )
		{
			bool * LignesAPermuter = ( bool* ) calloc( PositionActuelle + 1, sizeof( bool ) );
			bool* curLignesET = ( bool* ) malloc( ( PositionActuelle + 1 ) * sizeof( bool ) );
			bool Echangeable = true;
			const ConnectionPlace* tempGare;
			size_t i;
			size_t j;

			// Construction de l'ensemble des lignes a permuter
			LignesAPermuter[ PositionActuelle ] = true;
			size_t __i=0;
			for ( JourneyVector::const_iterator it = jv.begin(); it != jv.end(); ++it, ++__i )
			{
				OrdrePAConstruitLignesAPermuter( pl, *it, curLignesET, PositionActuelle );
				for ( i = PositionActuelle; i > PositionGareSouhaitee; i-- )
					if ( curLignesET[ i ] && LignesAPermuter[ i ] )
						break;
				for ( ; i > PositionGareSouhaitee; i-- )
					if ( curLignesET[ i ] )
						LignesAPermuter[ i ] = true;
			}

			// Tests d'�changeabilit� binaire
			// A la premiere contradiction on s'arrete
			__i=0;
			for ( JourneyVector::const_iterator it = jv.begin(); it != jv.end(); ++it, ++__i )
			{
				OrdrePAConstruitLignesAPermuter( pl, *it, curLignesET, PositionActuelle );
				i = PositionGareSouhaitee;
				for ( j = PositionGareSouhaitee; true; j++ )
				{
					for ( ; !LignesAPermuter[ i ]; i++ )
					{ }

					if ( i > PositionActuelle )
						break;

					if ( curLignesET[ i ] && curLignesET[ j ] && !LignesAPermuter[ j ] )
					{
						Echangeable = false;
						break;
					}
					i++;
				}
				if ( !Echangeable )
					break;
			}

			// Echange ou insertion
			if ( Echangeable )
			{
				for ( j = 0; true; j++ )
				{
					for ( i = j; !LignesAPermuter[ i ] && i <= PositionActuelle; i++ )
					{ }

					if ( i > PositionActuelle )
						break;

					LignesAPermuter[ i ] = false;

					tempGare = pl[ i ];
					for ( ; i > PositionGareSouhaitee + j; i-- )
						pl[ i - 1 ] = pl[ i ];
					pl[ i ] = tempGare;
				}
				return PositionGareSouhaitee + j;
			}
			else
				return OrdrePAInsere( pl, lll, pl[ PositionGareSouhaitee ], PositionActuelle + 1 );

		}

		size_t SchedulesTableInterfaceElement::OrdrePAInsere(PlaceList& pl, const LockedLinesList& lll, const synthese::env::ConnectionPlace* place, size_t Position )
		{
			// Saut de ligne v�rouill�e par un cheminement pi�ton
			for ( ; Position < lll.size() && lll[ Position ]; Position++ )
				;

			// D�calage des arr�ts suivants
			for ( size_t i = pl.size(); i > Position; i-- )
				pl[ i - 1 ] = pl[ i ];

			// Stockage de l'arr�t demand�
			pl[ Position ] = place;

			// Retour de la position choisie
			return Position;

		}

		void SchedulesTableInterfaceElement::OrdrePAConstruitLignesAPermuter( const PlaceList& pl, const synthese::routeplanner::Journey& __TrajetATester, bool* Resultat, size_t LigneMax )
		{
			int l = 0;
			const JourneyLeg* curET = (l >= __TrajetATester.getJourneyLegCount ()) ? NULL : __TrajetATester.getJourneyLeg (l);
			for (size_t i = 0; pl[ i ] != NULL && i <= LigneMax; i++ )
			{
				if ( curET != NULL && pl[ i ] == curET->getOrigin() ->getConnectionPlace() )
				{
					Resultat[ i ] = true;
					++l;
					curET = (l >= __TrajetATester.getJourneyLegCount ()) ? NULL : __TrajetATester.getJourneyLeg (l);
				}
				else
				{
					Resultat[ i ] = false;
				}
			}

		}

		bool SchedulesTableInterfaceElement::OrdrePARechercheGare( const PlaceList& pl, size_t& i, const synthese::env::ConnectionPlace* GareAChercher )
		{
			// Recherche de la gare en suivant � partir de la position i
			for ( ; i < pl.size() && pl[ i ] != NULL && pl[ i ] != GareAChercher; ++i );

			// Gare trouv�e en suivant avant la fin du tableau
			if ( i < pl.size() && pl[ i ] != NULL )
				return true;

			// Recherche de position ant�rieure � i
			for ( i = 0; i < pl.size() && pl[ i ] != NULL && pl[ i ] != GareAChercher; ++i );

			return i < pl.size() && pl[ i ] != NULL;

		}

		/** Build of the places list of a future schedule sheet corresponding to a journey vector.
			@author Hugues Romain
			@date 2001-2006

			Le but de la m�thode est de fournir une liste ordonn�e de points d'arr�t de taille minimale d�terminant les lignes du tableau de fiche horaire.

			Examples of results after journeys addings :

			Pas 0 : Service ABD (adding of B)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td></tr>
			<tr><td>B</td><td>X</td></tr>
			<tr><td>D</td><td>X</td></tr>
			</table>

			Pas 1 : Service ACD (adding of C)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td><td>X</td></tr>
			<tr><td>B</td><td>X</td><td|</td></tr>
			<tr><td>C</td><td>|</td><td>X</td></tr>
			<tr><td>D</td><td>X</td><td>X</td></tr>
			</table>

			Pas 2 : Service ACBD (change of the order authorized : descente de B au rang C+1)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td><td>X</td><td>X</td></tr>
			<tr><td>B</td><td>X</td><td>|</td><td>|</td></tr>
			<tr><td>C</td><td>|</td><td>X</td><td>X</td></tr>
			<tr><td>D</td><td>X</td><td>X</td><td>-</td></tr>
			</table>

			(permutation)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td><td>X</td><td>X</td></tr>
			<tr><td>C</td><td>|</td><td>X</td><td>X</td></tr>
			<tr><td>B</td><td>X</td><td>|</td><td>X</td></tr>
			<tr><td>D</td><td>X</td><td>X</td><td>X</td></tr>
			</table>

			Pas 3 : Service ABCD (change of the order refused : adding of an other C row)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
			<tr><td>C</td><td>|</td><td>X</td><td>X</td><td>|</td></tr>
			<tr><td>B</td><td>X</td><td>|</td><td>X</td><td>X</td></tr>
			<tr><td>C</td><td>|</td><td>|</td><td>|</td><td>X</td></tr>
			<tr><td>D</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
			</table>

			Pas 4 : Service AB->CD (service continu BC)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
			<tr><td>C</td><td>|</td><td>X</td><td>X</td><td>|</td><td>|</td></tr>
			<tr><td>B</td><td>X</td><td>|</td><td>X</td><td>X</td><td>V</td></tr>
			<tr><td>C</td><td>|</td><td>|</td><td>|</td><td>X</td><td>V</td></tr>
			<tr><td>D</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
			</table>

			Pas 5 : Service AED (E ins�r� avant B pour ne pas rompre la continuit� BC)

			<table class="Tableau" cellspacing="0" cellpadding="5">
			<tr><td>A</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
			<tr><td>C</td><td>|</td><td>X</td><td>X</td><td>|</td><td>|</td><td>|</td></tr>
			<tr><td>E</td><td>|</td><td>|</td><td>|</td><td>|</td><td>|</td><td>X</td></tr>
			<tr><td>B</td><td>X</td><td>|</td><td>X</td><td>X</td><td>V</td><td>|</td></tr>
			<tr><td>C</td><td>|</td><td>|</td><td>|</td><td>X</td><td>V</td><td>|</td></tr>
			<tr><td>D</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td><td>X</td></tr>
			</table>

			Pour chaque trajet, on proc�de donc par balayage dans l'ordre des gares existantes. Si la gare � relier n�est pas trouv�e entre la position de la gare pr�c�dente et la fin, deux solutions :
			- soit la gare n�est pr�sente nulle part (balayage avant la position de la pr�c�dente) auquel cas elle est cr��e et rajout�e � la position de la gare pr�c�dente + 1
			- soit la gare est pr�sente avant la gare pr�c�dente. Dans ce cas, on tente de descendre la ligne de la gare recherch�e au niveau de la position de la gare pr�c�dente + 1. On contr�le sur chacun des trajets pr�c�dents que la chronologie n'en serait pas affect�e. Si elle ne l'est pas, alors la ligne est descendue. Sinon une nouvelle ligne est cr��e.

			Contr�le de l'�changeabilit� :

			Soit \f$ \delta_{l,c}:(l,c)\mapsto\{{1\mbox{~si~le~trajet~}c\mbox{~dessert~la~ligne~}l\atop 0~sinon} \f$

			Deux lignes l et m sont �changeables si et seulement si l'ordre des lignes dont \f$ \delta_{l,c}=1 \f$ pour chaque colonne est respect�.

			Cet ordre s'exprime par la propri�t� suivante : Si \f$ \Phi \f$ est la permutation p�vue, alors

			<img width=283 height=27 src="interface.doxygen_fichiers/image008.gif">

			Il est donc n�cessaire � la fois de contr�ler la possibilit� de permutation, et de la d�terminer �ventuellement.

			Si <sub><img width=25 height=24
			src="interface.doxygen_fichiers/image009.gif"></sub>est la ligne de la gare
			pr�c�demment trouv�e, et <sub><img width=24 height=24
			src="interface.doxygen_fichiers/image010.gif"></sub>�l�emplacement de la gare
			souhait�e pour permuter, alors les permutations � op�rer ne peuvent concerner
			que des lignes comprises entre <sub><img width=24 height=24
			src="interface.doxygen_fichiers/image010.gif"></sub>�et <sub><img width=25
			height=24 src="interface.doxygen_fichiers/image009.gif"></sub>. En effet, les
			autres lignes n�influent pas.</p>


			En premier lieu il est n�cessaire de d�terminer l�ensemble
			des lignes � permuter. Cet ensemble est construit en explorant chaque colonne.
			Si <sub><img width=16 height=24 src="interface.doxygen_fichiers/image011.gif"></sub>�est
			l�ensemble des lignes � permuter pour assurer l�int�grit� des colonnes <sub><img
			width=36 height=27 src="interface.doxygen_fichiers/image012.gif"></sub>, on
			peut d�finir cet ensemble en fonction du pr�c�dent <sub><img width=25
			height=24 src="interface.doxygen_fichiers/image013.gif"></sub>&nbsp;: <sub><img
			width=308 height=35 src="interface.doxygen_fichiers/image014.gif"></sub>

			Le but �tant de faire descendre la ligne <sub><img width=24
			height=24 src="interface.doxygen_fichiers/image010.gif"></sub>�vers <sub><img
			width=25 height=24 src="interface.doxygen_fichiers/image009.gif"></sub>, les
			lignes appartenant � L doivent �tre �changeables avec� les positions <sub><img
			width=216 height=27 src="interface.doxygen_fichiers/image015.gif"></sub>.
			L�ensemble de ces tests doit �tre r�alis�. Au moindre �chec, l�ensemble de la
			permutation est rendu impossible.

			L��changeabilit� binaire entre deux lignes l et m revient �
			contr�ler la propri�t�&nbsp;<sub><img width=89 height=28
			src="interface.doxygen_fichiers/image016.gif"></sub>.

			L��changeabilit� totale s��crit donc <sub><img width=145
			height=28 src="interface.doxygen_fichiers/image017.gif"></sub>

			L�algorithme est donc le suivant&nbsp;:

			- Construction de L
			- Contr�le d��changeabilit� binaire pour chaque �l�ment de L avec
			sa future position
			- Permutation

			<b>Echange</b>&nbsp;:

			Exemple d��change&nbsp;:

			<table class=MsoNormalTable border=1 cellspacing=0 cellpadding=0 width=340
			style='width:254.95pt;margin-left:141.6pt;border-collapse:collapse;border:
			none'>
			<tr>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>X</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>X</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>X</p>
			</td>
			</tr>
			</table>

			<span style='position:relative;z-index:16'><span
			style='left:0px;position:absolute;left:398px;top:-1px;width:67px;height:53px'><img
			width=67 height=53 src="interface.doxygen_fichiers/image018.gif"></span></span><span
			style='position:relative;z-index:13'><span style='left:0px;position:absolute;
			left:371px;top:-1px;width:67px;height:52px'><img width=67 height=52
			src="interface.doxygen_fichiers/image019.gif"></span></span><span
			style='position:relative;z-index:12'><span style='left:0px;position:absolute;
			left:349px;top:-1px;width:62px;height:53px'><img width=62 height=53
			src="interface.doxygen_fichiers/image020.gif"></span></span><span
			style='position:relative;z-index:11'><span style='left:0px;position:absolute;
			left:322px;top:-1px;width:69px;height:52px'><img width=69 height=52
			src="interface.doxygen_fichiers/image021.gif"></span></span><span
			style='position:relative;z-index:10'><span style='left:0px;position:absolute;
			left:269px;top:-1px;width:97px;height:53px'><img width=97 height=53
			src="interface.doxygen_fichiers/image022.gif"></span></span><span
			style='position:relative;z-index:14'><span style='left:0px;position:absolute;
			left:455px;top:-1px;width:37px;height:51px'><img width=37 height=51
			src="interface.doxygen_fichiers/image023.gif"></span></span><span
			style='position:relative;z-index:15'><span style='left:0px;position:absolute;
			left:482px;top:-1px;width:33px;height:51px'><img width=33 height=51
			src="interface.doxygen_fichiers/image024.gif"></span></span><span
			style='position:relative;z-index:6'><span style='left:0px;position:absolute;
			left:248px;top:-1px;width:262px;height:53px'><img width=262 height=53
			src="interface.doxygen_fichiers/image025.gif"></span></span><span
			style='position:relative;z-index:5'><span style='left:0px;position:absolute;
			left:221px;top:-1px;width:206px;height:53px'><img width=206 height=53
			src="interface.doxygen_fichiers/image026.gif"></span></span><span
			style='position:relative;z-index:7'><span style='left:0px;position:absolute;
			left:242px;top:-1px;width:97px;height:52px'><img width=97 height=52
			src="interface.doxygen_fichiers/image027.gif"></span></span><span
			style='position:relative;z-index:9'><span style='left:0px;position:absolute;
			left:216px;top:-1px;width:96px;height:52px'><img width=96 height=52
			src="interface.doxygen_fichiers/image028.gif"></span></span><span
			style='position:relative;z-index:8'><span style='left:0px;position:absolute;
			left:193px;top:-1px;width:96px;height:52px'><img width=96 height=52
			src="interface.doxygen_fichiers/image029.gif"></span></span><span
			style='position:relative;z-index:4'><span style='left:0px;position:absolute;
			left:194px;top:-1px;width:103px;height:52px'><img width=103 height=52
			src="interface.doxygen_fichiers/image030.gif"></span></span>

			<table>
			<tr>
			<td>
			<p class=Tableau>X</p>
			</td>
			<td>
			<p class=Tableau>X</p>
			</td>
			<td>
			<p class=Tableau>X</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			<td>
			<p class=Tableau>-</p>
			</td>
			</tr>
			</table>

		*/
		SchedulesTableInterfaceElement::PlaceList SchedulesTableInterfaceElement::getStopsListForScheduleTable( const JourneyVector& jv )
		{
			// Variables locales
			size_t i;
			size_t dernieri;

			// Allocation
			LockedLinesList lll;
			PlaceList pl;

			// Horizontal loop
			for ( JourneyVector::const_iterator it = jv.begin(); it != jv.end(); ++it )
			{
				i = 0;
				dernieri = -1;

				// Vertical loop
				for (int l = 0; l < it->getJourneyLegCount (); ++l)
				{
					const JourneyLeg * curET = it->getJourneyLeg (l);

					// Search of the place from the preceding one
					if ( OrdrePARechercheGare( pl, i, curET->getOrigin()->getConnectionPlace() ) )
					{
						if ( i < dernieri )
							i = OrdrePAEchangeSiPossible( jv, pl, lll, dernieri, i );
					}
					else
					{
						i = OrdrePAInsere( pl, lll, curET->getOrigin() ->getConnectionPlace(), dernieri + 1 );
					}

					dernieri = i;
					i++;

					// Controle gare suivante pour trajet a pied
					if ( /* MJ que deviennent les lignes � pied ??? curET->getLigne() ->Materiel() ->Code() == MATERIELPied && */   
						pl[ i ] != curET->getDestination() ->getConnectionPlace() && (l != it->getJourneyLegCount ()-1) )
					{
						if ( OrdrePARechercheGare( pl, i, curET->getDestination() ->getConnectionPlace() ) )
						{
							OrdrePAEchangeSiPossible(jv, pl, lll, dernieri, i );
							i = dernieri + 1;
						}
						else
						{
							i = dernieri + 1;
							OrdrePAInsere( pl, lll, curET->getDestination() ->getConnectionPlace(), i );
						}
						lll.insert( lll.begin() + i, true );
					}
				}
			}

			// Ajout de la destination finale en fin de tableau
			if ( jv.size() > 0 )
				pl.push_back( jv[0].getDestination()->getConnectionPlace() );

			return pl;
		}




	
		/*! \brief Cr�ation des niveaux d'alerte des trajets en fonction des donn�es lignes et arr�ts
		\author Hugues Romain
		\date 2005
		*/
/*		void cTrajets::GenererNiveauxEtAuMoinsUneAlerte()
		{
			// Variables locales
			bool __AuMoinsUneAlerte = false;

			// Calcul des niveaux d'alerte pour chaque trajet et collecte du resultat
			for ( int __i = 0; __i < Taille(); __i++ )
			{
				__AuMoinsUneAlerte = getElement( __i ).getMaxAlarmLevel () || __AuMoinsUneAlerte;
			}

			// Stockage du resultat final de la liste de trajets sous format texte pour 
			// exploitabilite directe par module d'interface
			if ( __AuMoinsUneAlerte )
				_AuMoinsUneAlerte = "1";
			else
				_AuMoinsUneAlerte.clear();
		}
*/


	}
}