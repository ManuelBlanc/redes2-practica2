#!/usr/bin/perl
use strict;
use warnings;
use diagnostics;

use Text::Template;
use File::Basename;

my $template = Text::Template->new(TYPE => 'STRING', SOURCE => <<'END_TEMPLATE');

{uc $function_name}(3)
{'=' x (3 + length $function_name)  }
:doctype: manpage
:format: manpage

NOMBRE
------
*{$function_name}* - Descripción breve de la función

SYNOPSIS
--------
*#include <{$file}>*

_{$return_type}_ *{$function_name}*({
	join ', ', (map { "_$_->{both}_"} @args)
});

DESCRIPTION
-----------
{ (@args+1) ? join "\n", (map { "_$_->{both}_:: Descripción del parametro" } (grep { $_->{type} ne "void" }, @args)) : "No tiene parámetros de entrada." }

RETURN VALUE
------------
{
	if ($return_type eq 'void') {
		"No devuelve ningún valor ni código de error."
	}
	else {
		sprintf "%s\n%s",
			"*OK*::  Ha funcionado",
			"*ERR*:: Ha ocurrido un error (argumento erroneo; fallo de memoria; error inesperado)."
	}
}

SEE ALSO
--------
otra_funcion(3)

AUTHORS
-------
Ana Alonso <ana.alonsoa@estudiante.uam.es>, +
Manuel Blanc <manuel.blanc@estudiante.uam.es>

END_TEMPLATE

while (<>) {
	next unless /^(\w+[\s*]+)(\w+)\((.*?)\);/;
	my ($return_type, $function_name, $args_str) = ($1, $2, $3);
	my @args;

	print "Procesando $_";

	$return_type =~ s/\s+//g;

	foreach (split /\s*,\s*/, $args_str) {
		my ($type, $name) = $_ =~ /(\w+[\s*]+|\.\.\.|void)(\w*)/;
		$type =~ s/\s+//g;
		push @args, {type => $type, name => $name, both => $name ? "$type $name" : "$type"};
	}

	open(my $fh, ">", "$function_name.3.txt") or die "cannot open > $function_name.3.txt: $!";
	$template->fill_in(OUTPUT => $fh, HASH =>
	{
		file         	=> basename($ARGV),
		return_type  	=> $return_type,
		function_name	=> $function_name,
		args         	=> [@args],
	});
	close($fh) || warn "close failed: $!";
}

=pod

int main(int argc, char** argv);

=cut
