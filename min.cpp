#include <cstdio>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include "src/pugixml.hpp"

using namespace std;

vector < int > t1_state, t2_state, f_state;
vector < char > r_state;
set    < int > entrada_state;
int **tMap, **ptMap;
int inicial_;
long int r;
long int todos_estados = 0;
long int estadosfinais = 0;
int estado_inicial = 0;
long int estados_n_finais;
long int *P;
FILE *fp;

void
cria_xml ( char *saida_xml)
{

  fstream fs;
  fs.open (saida_xml, ios::out);
  if (not fs.is_open ())
  {
      cout << "Erro no arquivo de saida" << endl;
      exit ( EXIT_FAILURE);
  }

  fs << "<structure>" << endl;
  fs << "<type>" << "fa" << "</type>" << endl;
  fs << "<automaton>" << endl;

  for (auto i = begin (entrada_state); i != end (entrada_state); i++)
    {
      fs << "<state id =" << '"' << *i << '"' << " name=" << '"' << "q" << *i
	<< '"' << '>' << endl;
      if (*i == estado_inicial)
	{
	  fs << "<initial/>" << endl;
	}

    for (auto j:f_state)
	{
	  if (j == *i)
	    {
	      fs << "<final/>" << endl;

	    }
	}
      fs << "</state>" << endl;

    }

  for (size_t i = 0; i < t1_state.size (); i++)
    {
      fs << "<transition>" << endl;
      fs << "<from>" << t1_state[i] << "</from>" << endl;
      fs << "<to>" << t2_state[i] << "</to>" << endl;
      fs << "<read>" << r_state[i] << "</read>" << endl;
      fs << "</transition>" << endl;
    }

  fs << "</automaton>" << endl;
  fs << "</structure>" << endl;
  fs.close ();

}

void
entrada_xml ( char *arquivo_xml)
{
  pugi::xml_document doc;
  string nome;

  if (!doc.load_file (arquivo_xml))
  {
    exit (EXIT_FAILURE);
  }
  fstream f_temp ("tmp.txt", ios::out | ios::in | ios::app);
  fstream fe;
  fe.open ("entrada.txt", ios::out);

  if (not fe.is_open ())
    {
      cout << "Erro no arquivo de saida" << endl;
      exit (EXIT_FAILURE);
    }


  pugi::xml_node tools = doc.child ("structure").child ("automaton");

  for (pugi::xml_node tool:tools.children ("state"))
    {
    for (pugi::xml_attribute attr:tool.attributes ())
	{
	  f_temp << attr.value () << " ";
	}

    for (pugi::xml_node child:tool.children ())
	{
	  string s = child.name ();
	  if (s == "final" or s == "initial")
	    {
	      f_temp << " " << child.name ();
	    }
	}

      f_temp << std::endl;
    }

  f_temp.seekg (0);


  for (; getline (f_temp, nome);)
    {
      if (nome.find ("initial") != string::npos)
	{
	  fe << nome[0] << endl;
	  break;
	}

    }

  string a;
  string b = "";

  f_temp.seekg (0);
  for (; getline (f_temp, nome);)
    {
      if (nome.find ("final") != string::npos)
	{
	  a.push_back (nome[0]);

	}

    }

  for (size_t i = 0; i < a.length (); i++)
    {
      b.append (1, a[i]);
      b.append (1, ' ');

    }

  b.pop_back ();
  fe << b;
  fe << endl;

  for (auto transicao = tools.child ("transition"); transicao;
       transicao = transicao.next_sibling ("transition"))
    {
      fe << transicao.child_value ("from") << " " << transicao.
	child_value ("read") << " " << transicao.child_value ("to") << endl;
    }

  f_temp.close ();
  remove ("tmp.txt");
  fe.close ();

}

void
dfs (int v)
{
  r |= (1 << v);

  for (int i = 0; i < 26; i++)
    {
      if ((tMap[v][i] != -1)
	  && ((r & (1 << tMap[v][i])) == 0))
	{
	  dfs (tMap[v][i]);
	}
    }
}


int
main (int argc, char *argv[])
{
  
  if ( argc != 3 )
  {
      cout << "entrada errada" << endl;
      exit(-1);
  }

  entrada_xml ( argv[1] );

  fp = fopen ("entrada.txt", "r");

  if (!fp)
    {
      cerr << "Erro no arquivo de entrada" << endl;
      return EXIT_FAILURE;
    }

  tMap =  (int **) malloc (64 * sizeof (int *));
  ptMap = (int **) malloc (64 * sizeof (int *));
  P = (long int *) malloc (64 * sizeof (long int));
  for (int i = 0; i < 64; i++)
    {
      P[i] = 0;
      tMap[i] = (int *) malloc (26 * sizeof (int));
      ptMap[i] = (int *) malloc (26 * sizeof (int));
      for (int j = 0; j < 26; j++)
	{
	  tMap[i][j] = -1;
	  ptMap[i][j] = -1;
	}
    }

  char buff[125];
  fgets (buff, sizeof (buff), fp);
  char *p = strtok (buff, " ");
  inicial_ = atoi (p);

  fgets (buff, sizeof (buff), fp);
  p = strtok (buff, " ");
  while (p != NULL)
    {
      int estado = atoi (p);
      estadosfinais |= 1 << (estado);
      p = strtok (NULL, " ");
    }

  int from, to;
  char read;
  while (fscanf (fp, "%d %c %d", &from, &read, &to) != EOF)
    {
      tMap[from][read - 'a'] = to;

      todos_estados |= (1 << from);
      todos_estados |= (1 << to);
    }

  r = 0;
  dfs (inicial_);

  todos_estados &= r;
  estadosfinais &= r;



  estados_n_finais = todos_estados & ~estadosfinais;
  P[0] = estadosfinais;
  P[1] = estados_n_finais;

  int NPI = 2;

  for (int i = 0; i < 64; i++)
    {

      long int NP = 0;

      if (P[i] == 0)
	{
	  break;
	}

      for (int j = 63; j >= 0; j--)
	{

	  long int s_estado = (long int) 1 << j;

	  if ((P[i] & (s_estado)) != 0)
	    {

	      ptMap[i] = tMap[j];

	      for (int k = j - 1; k >= 0; k--)
		{
		  long int outro_estado = (long int) 1 << k;

		  if ((P[i] & (outro_estado)) != 0)
		    {

		      for (int l = 0; l < 26; l++)
			{
			  int SN = -1;
			  int ON = -1;

			  for (int m = 0; m < NPI; m++)
			    {
			      if ((P[m] & (1 << tMap[j][l])) != 0)
				{
				  SN = m;
				}
			      if ((P[m] & (1 << tMap[k][l])) != 0)
				{
				  ON = m;
				}
			    }
			      
			  if (tMap[j][l] != tMap[k][l]
			      && (SN != ON))
			    {
			      P[i] &= ~(1 << k);
			      NP |= (1 << k);
			      break;
			    }
			}
		    }
		}
	      break;
	    }
	}
     
      if (NP != 0)
	{
	  P[NPI] = NP;
	  NPI++;
	}
    }

  for (int i = 0; i < NPI; i++)
    {
      if ((P[i] & (1 << inicial_)) != 0)
	{
	  estado_inicial = i;
	  break;
	}
    }

  for (int i = 0; i < NPI; i++)
    {
      if ((P[i] & estadosfinais) != 0)
	{
	  f_state.push_back (i);
	}
    }

  for (int i = 0; i < NPI; i++)
    {
      for (int j = 0; j < 26; j++)
	{
	  if (ptMap[i][j] != -1)
	    {
	      for (int k = 0; k < NPI; k++)
		{
		  if ((P[k] & (1 << ptMap[i][j])) != 0)
		    {
		      t1_state.push_back (i);
		      r_state.push_back (j + 'a');
		      t2_state.push_back (k);
		      entrada_state.insert (i);
		      entrada_state.insert (k);

		    }
		}
	    }
	}
    }

  cria_xml (argv[2]);
  remove ("entrada.txt");

  return 0;

}
