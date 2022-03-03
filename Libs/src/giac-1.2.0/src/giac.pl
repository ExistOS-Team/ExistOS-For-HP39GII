#!/usr/bin/perl

printf STDOUT "Content-type: text/xml\n\n";

# les donnees sont envoyees par methode GET
# donc on recupere les donnees dans la variable 
# d'environnement QUERY_STRING
$buffer=$ENV{"QUERY_STRING"};

# on split la chaine de donnees en des paires name=value
local(@champs) = split(/&/, $buffer);
local($donnees) = "";

# recuperation et mise en forme des donnees
# on parcourt la liste des paires name=value
my $param;
foreach $i (0 .. $#champs) {
  # On convertit les plus en espaces
  $champs[$i] =~ s/\+/ /g;
    
  # On separe chaque champ en une cle et sa valeur
  ($key, $val) = split(/=/,$champs[$i],2); 
    
  # On convertit les %XX de leur valeur hexadecimale en alphanumerique
  $key =~ s/%(..)/pack("c",hex($1))/ge;
  $val =~ s/%(..)/pack("c",hex($1))/ge;

  # on affiche le resultat
 # printf STDOUT "<STRONG>%s::</STRONG>%s\n",$key,$val;
  if ($key eq "giac"){
    $param ="\"".$val."\"";
  }
}
my $string1;
my $string2;

# ------------ mettre le bon répertoire ----------------
$cas="./cas2html";

open(CAS, $cas." -m ".$param." |");
while (<CAS>) {
  $string1=$_;
}
close(CAS);

open(CAS, $cas." -m -e ".$param." |");
while (<CAS>) {
  $string2=$_;
}
close(CAS);

# affichage du code XML

printf STDOUT "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n";

printf STDOUT "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN\"\n";
printf STDOUT "      \"http://www.w3.org/TR/MathML2/dtd/xhtml-math11-f.dtd\">\n";
#printf STDOUT " [<!ENTITY mathml \"http://www.w3.org/1998/Math/MathML\">]>\n";
#printf STDOUT "<?xml-stylesheet type=\"text/xsl\" href=\"pmathml.xsl\"?>\n";
printf STDOUT "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
printf STDOUT "<head>\n";
printf STDOUT  "<title>Giac en ligne</title>\n";
printf STDOUT "</head>\n";
printf STDOUT "<body>\n";
printf STDOUT "<div style=\"text-align: center\">";
printf STDOUT "<h3>Exemples d'instruction pour XCAS :</h3>\n";
printf STDOUT " <table width=\"99%\" border=\"1\" ><col width=\"33%\"/><col width=\"33%\"/><col width=\"33%\"/><tbody>";

printf STDOUT "<tr><td>Développer et réduire :<br/>normal((x-1)*(x+2))</td><td>Factoriser :<br/>factor(x^2+x-2)</td><td>Résoudre une équation :<br/>solve(x^4-1=3)</td></tr>";

printf STDOUT "<tr><td>Dériver une fonction :<br/>diff((x-a)/(x+a),x)</td><td>Trouver une primitive :<br/>integrate(t^2,t)</td><td>Calculer une intégrale :<br/>integrate(x^2,x,1,2)</td></tr>";

printf STDOUT "<tr><td>Calculer une limite en +infini:<br/>limit(1/x, x=+infinity)</td><td>Calculer une limite en 0<sup>+</sup>:<br/>limit(1/x, x=0, 1)</td><td>Calculer une limite en 0<sup>-</sup>:<br/>limit(1/x, x=0, -1)</td></tr>";

printf STDOUT "<tr><td>Résoudre l'éq diff à coeff const :<br/>desolve([y''+y=cos(x),y(0)=1],y)</td><td>Calculer une somme :<br/>sum(1/n^2, n, 1, 10)</td><td>Multiplier deux matrices :<br/>[[1,2],[3,4]] * [[5,6],[7,8]]</td></tr>";

printf STDOUT "</tbody></table>";
printf STDOUT "Documentation complete <a href=\"http://www-fourier.ujf-grenoble.fr/~parisse/casflan/casflan.html\">ici</a>\n";
printf STDOUT "<h3>Entrez ici une instruction pour XCAS :\n";
printf STDOUT "<form action=\"http://localhost/cgi-bin/giac.pl\" method=\"get\">\n";
if ($param ne ""){
    printf STDOUT "<input type=\"text\" name=\"giac\" value=".$param." size=\"33\" /> \n";
}
else {
    printf STDOUT "<input type=\"text\" name=\"giac\" size=\"33\" /> \n";
}
printf STDOUT "<input type=\"submit\" value=\"Calculer\" />\n";
printf STDOUT "<input type=\"reset\" value=\"Reset\" />\n";
printf STDOUT "</form>\n</h3>\n";
if ($param ne ""){
  printf STDOUT "Instruction entrée : ";
  printf STDOUT  $param."\n<p>";
  printf STDOUT "<math mode=\"display\" xmlns=\"http://www.w3.org/1998/Math/MathML\">\n";
  printf STDOUT  $string1." <mo>=</mo> ".$string2;
  printf STDOUT "</math></p>\n";
}
printf STDOUT "</div>";
printf STDOUT "</body>";
printf STDOUT "</html>";

