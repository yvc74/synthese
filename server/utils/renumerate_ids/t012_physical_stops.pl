#!/usr/bin/perl

use strict;
use DBI;

my $node_id_cible = 50;

my $dbh = DBI->connect(          
    "dbi:SQLite:dbname=config.db3", 
    "",                          
    "",                          
    { RaiseError => 1 },         
) or die $DBI::errstr;

my $sth = $dbh->prepare("SELECT id FROM t012_physical_stops ORDER BY id;");
$sth->execute();

# Run on id of t012_physical_stops looking for those with node_id 0 or 1
# In the same time we look for the highest id with node_id of our database
# it is the last with node_id because of ORDER BY in the request
my @tab_id_to_change = ();
my $num_id_to_change = 0;
my $last_id_of_this_node = 0;
while (my $ids = $sth->fetchrow_hashref())
{
	my $id=$$ids{'id'};
	my $id_hex = sprintf("%x", $id);
	my $node_id_hex = substr $id_hex, 3, 2; # 3 because table code < 16
	my $node_id = sprintf hex $node_id_hex;
	if ($node_id == 0 || $node_id == 1)
	{
		$tab_id_to_change[$num_id_to_change] = $id;
		$num_id_to_change++;
	}
	if ($node_id == $node_id_cible)
	{
		$last_id_of_this_node = $id;
	}
}
$sth->finish();

if ($last_id_of_this_node == 0)
{
	# There is no id with node_id in this table !
	my $table_id_hex = sprintf("%x", 12);
	$table_id_hex .= "00";
	my $node_id_hex = sprintf("%x", $node_id_cible);
	$node_id_hex .= "00";
	my $last_id_of_this_node_hex = $table_id_hex.$node_id_hex."000000";
	$last_id_of_this_node = sprintf hex $last_id_of_this_node_hex;
}

print "We found $num_id_to_change physical_stops to change\n";
print "The highest id with node $node_id_cible is $last_id_of_this_node\n";
sleep(5);

# New id table
my $cpt=0;
my @tab_new_id = ();
for ($cpt;$cpt<$num_id_to_change;$cpt++)
{
	$last_id_of_this_node++;
	$tab_new_id[$cpt] = $last_id_of_this_node;
}

# Requests to update the database written in a file because
# - we will be able to check what is really done in our database
# - sqlite perl driver does not manage with spatial tables

open(FILE,">replace_id_t012_physical_stops.sql") or die"open: $!";
$cpt=0;
my %t016_scheduled_services_stops = ();
my %t070_compositions_served_vertices = ();
my %t041_display_screens_physical_stops_ids = ();
my %t052_timetables_physical_authorized_physical_stops = ();
for ($cpt;$cpt<$num_id_to_change;$cpt++)
{
	print "Update $cpt on $num_id_to_change (".$tab_id_to_change[$cpt]." => ".$tab_new_id[$cpt].")\n";
	#0.  Column id of t012_physical_stops
	my $sql = "UPDATE t012_physical_stops SET id = $tab_new_id[$cpt] WHERE id = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#1. Column start_physical_stop of t066_junctions
	my $sql = "UPDATE t066_junctions SET start_physical_stop = $tab_new_id[$cpt] WHERE start_physical_stop = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#2. Column end_physical_stop of t066_junctions
	my $sql = "UPDATE t066_junctions SET end_physical_stop = $tab_new_id[$cpt] WHERE end_physical_stop = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#3. Column physical_stop_id of t010_line_stops
	my $sql = "UPDATE t010_line_stops SET physical_stop_id = $tab_new_id[$cpt] WHERE physical_stop_id = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#4. Column stops of t016_scheduled_services (to split)
	$sth = $dbh->prepare("SELECT id, stops FROM t016_scheduled_services WHERE stops LIKE '%".$tab_id_to_change[$cpt]."%';");
	$sth->execute();
	while (my $result = $sth->fetchrow_hashref())
	{
		my $id_scheduled_service=$$result{'id'};
		my $stops=$$result{'stops'};
		my $stops_copy = $stops;
		if( exists( $t016_scheduled_services_stops{$id_scheduled_service} ) )
		{
			$stops_copy = $t016_scheduled_services_stops{$id_scheduled_service};
		}
		$stops_copy =~ s/$tab_id_to_change[$cpt]/$tab_new_id[$cpt]/g;
		$t016_scheduled_services_stops{$id_scheduled_service}=$stops_copy;
	}
	$sth->finish();
	#5. Column stop_id of t080_dead_runs
	my $sql = "UPDATE t080_dead_runs SET stop_id = $tab_new_id[$cpt] WHERE stop_id = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#6. Column served_vertices of t070_compositions (to split)
	$sth = $dbh->prepare("SELECT id, served_vertices FROM t070_compositions;");
	$sth->execute();
	while (my $result = $sth->fetchrow_hashref())
	{
		my $id_composition=$$result{'id'};
		my $served_vertices=$$result{'served_vertices'};
		my $served_vertices_copy = $served_vertices;
		if( exists( $t070_compositions_served_vertices{$id_composition} ) )
		{
			$served_vertices_copy = $t070_compositions_served_vertices{$id_composition};
		}
		$served_vertices_copy =~ s/$tab_id_to_change[$cpt]/$tab_new_id[$cpt]/g;
		$t070_compositions_served_vertices{$id_composition}=$served_vertices_copy;
	}
	$sth->finish();
	#7. Column stop_point_id of t072_vehicle_positions
	my $sql = "UPDATE t072_vehicle_positions SET stop_point_id = $tab_new_id[$cpt] WHERE stop_point_id = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#8. Column stop_point_location of t041_display_screens
	my $sql = "UPDATE t041_display_screens SET stop_point_location = $tab_new_id[$cpt] WHERE stop_point_location = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#9. Column physical_stops_ids of t041_display_screens (to split)
	$sth = $dbh->prepare("SELECT id, physical_stops_ids FROM t041_display_screens;");
	$sth->execute();
	while (my $result = $sth->fetchrow_hashref())
	{
		my $id_display_screen=$$result{'id'};
		my $physical_stops_ids=$$result{'physical_stops_ids'};
		my $physical_stops_ids_copy = $physical_stops_ids;
		if( exists( $t041_display_screens_physical_stops_ids{$id_display_screen} ) )
		{
			$physical_stops_ids_copy = $t041_display_screens_physical_stops_ids{$id_display_screen};
		}
		$physical_stops_ids_copy =~ s/$tab_id_to_change[$cpt]/$tab_new_id[$cpt]/g;
		$t041_display_screens_physical_stops_ids{$id_display_screen}=$physical_stops_ids_copy;
	}
	$sth->finish();
	#10. Column object_id of t040_alarm_object_links
	my $sql = "UPDATE t040_alarm_object_links SET object_id = $tab_new_id[$cpt] WHERE object_id = $tab_id_to_change[$cpt]";
	print FILE $sql.";\n";
	#11. Column authorized_physical_stops of t052_timetables
	$sth = $dbh->prepare("SELECT id, authorized_physical_stops FROM t052_timetables;");
	$sth->execute();
	while (my $result = $sth->fetchrow_hashref())
	{
		my $id_timetable=$$result{'id'};
		my $authorized_physical_stops=$$result{'authorized_physical_stops'};
		my $authorized_physical_stops_copy = $authorized_physical_stops;
		if( exists( $t052_timetables_physical_authorized_physical_stops{$id_timetable} ) )
		{
			$authorized_physical_stops_copy = $t052_timetables_physical_authorized_physical_stops{$id_timetable};
		}
		$authorized_physical_stops_copy =~ s/$tab_id_to_change[$cpt]/$tab_new_id[$cpt]/g;
		$t052_timetables_physical_authorized_physical_stops{$id_timetable}=$authorized_physical_stops_copy;
	}
	$sth->finish();
}

foreach my $id_t016_scheduled_services ( keys %t016_scheduled_services_stops ) {
	my $sql = "UPDATE t016_scheduled_services SET stops = '$t016_scheduled_services_stops{$id_t016_scheduled_services}' WHERE id = $id_t016_scheduled_services";
	print FILE $sql.";\n";
}
foreach my $id_t070_compositions ( keys %t070_compositions_served_vertices ) {
	my $sql = "UPDATE t070_compositions SET served_vertices = '$t070_compositions_served_vertices{$id_t070_compositions}' WHERE id = $id_t070_compositions";
	print FILE $sql.";\n";
}
foreach my $id_t041_display_screens ( keys %t041_display_screens_physical_stops_ids ) {
	my $sql = "UPDATE t041_display_screens SET physical_stops_ids = '$t041_display_screens_physical_stops_ids{$id_t041_display_screens}' WHERE id = $id_t041_display_screens";
	print FILE $sql.";\n";
}
foreach my $id_t052_timetables ( keys %t052_timetables_physical_authorized_physical_stops ) {
	my $sql = "UPDATE t052_timetables SET authorized_physical_stops = '$t052_timetables_physical_authorized_physical_stops{$id_t052_timetables}' WHERE id = $id_t052_timetables";
	print FILE $sql.";\n";
}

$dbh->disconnect();