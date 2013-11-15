<?php

if ( ! isset( $argv[1] ) ) {

	echo "Usage: php alive.php file\n";
	exit;
}

$lines = file($argv[1]);
$lines = array_unique($lines);

$output = '';
echo count($lines)." lines\n\n";

foreach ( $lines as $line ) {
	$o = array();
	exec("dig +time=1 +tries=1 ANY isc.org @$line",$o);
	if ( count($o) > 10 ) {
		echo '°';
		$output .= trim($line) . "\n";
	} else {
		echo '.';
	}
}

file_put_contents($argv[1].'.alive',$output);
