#include <cmath>
#include <iostream>

// calculando a função sigmasdLinha

double calculaSigmasdLinha(double Es, double es, double fyd) {
  
  // 1 - Trabalhar com o valor absoluto da deformação
  double ess = std::abs(es);

  // 2 - Deformação de escoamento de cálculo do aço
  double eyd = fyd / Es;

  //Cálculo da tensão
  double sigmasd;
  if (ess < eyd) {
    sigmasd = Es * ess;
  } else {
    sigmasd = fyd;
  }
  
  //Acertando o sinal da tensão
  if (es < 0) {
    sigmasd = -sigmasd;
  }
  return sigmasd; 
}

int main () {
  // 1 - Entrada de dados

  // Ler as propriedades dos Materiais

  struct PropriedadesMateriais {
    double fck; // MPa
    double fyk; //MPa
    double Es; //GPa
  };

  PropriedadesMateriais propriedadesMateriais = {70, 500, 200};

  // Ler os coeficientes parciais de segurança

  struct CoeficientesSeguranca {
    double gamac; 
    double gamas;
    double gamaf;
  };

  CoeficientesSeguranca coeficienteSeguranca = {1.4, 1.15, 1.4};

  // Ler o coeficiente de redistribuição dos momentos

  const double beta = 1;

  // Ler as dimensões da seção

  struct DimensoesSecao {
    double b; // cm
    double h; // cm
    double d; // cm
    double dLinha; //cm
  };

  DimensoesSecao dimensoesSecao = {15, 40, 36, 4};

  // Ler o momento fletor de serviço

  double Mk = 70; // kN.m

  // Fim da entrada de dados e início dos cálculos

  // 2 - Parâmetros do diagrama retangular para o concreto e profundidade da linha neutra
  double lambda, alfac, eu, qsiLimite;
  if (propriedadesMateriais.fck < 50) {
    lambda = 0.8;
    alfac = 0.85;
    eu = 3.5;
    qsiLimite = 0.8 * beta - 0.35;
  } else { 
    lambda = 0.8 - ((propriedadesMateriais.fck - 50) / 400);
    alfac = 0.85 * (1 - ((propriedadesMateriais.fck - 50) / 200));
    eu = 2.6 + 35 * std::pow(((90 - propriedadesMateriais.fck) / 100), 4);
    qsiLimite = 0.8 * beta - 0.45;
  }

  // 3- Conversão das unidades para kN e cm 
  Mk *= 100;
  propriedadesMateriais.fck /= 10;
  propriedadesMateriais.fyk /= 10;
  propriedadesMateriais.Es *= 100;

  // 4- Resistências e momento de cálculo
  double fcd = propriedadesMateriais.fck / coeficienteSeguranca.gamac;
  double sigmacd = alfac * fcd;
  double fyd = propriedadesMateriais.fyk / coeficienteSeguranca.gamas;
  double Md = coeficienteSeguranca.gamaf * Mk;

  // 5- Parâmetros Geométricos
  const double delta = dimensoesSecao.dLinha / dimensoesSecao.d;

  // 6- Momento Limite
  const double miLimite = lambda * qsiLimite * (1 - (0.5 * lambda * qsiLimite));

  // 7 - Momento Reduzido Solicitante
  const double mi = Md / (dimensoesSecao.b * dimensoesSecao.d * dimensoesSecao.d * sigmacd);

  // 8 - Solução com armadura simples
  double qsi, As, AsLinha, esLinha;
  if (mi < miLimite) {
    qsi = (1 - std::sqrt(1 - (2 * mi))) / lambda;
    As = lambda * qsi * dimensoesSecao.b * dimensoesSecao.d * (sigmacd / fyd);
    AsLinha = 0;
  } else {
    // Armadura dupla
    const double qsia = eu / (eu + 10);
    if (qsiLimite < qsia) {
      throw std::runtime_error("Você deve aumentar as dimesões da seção transversal");
    }
    if (qsiLimite <= delta) {
      throw std::runtime_error("Armadura de compressão está tracionada");
    }
    esLinha = eu * ((qsiLimite - delta) / qsiLimite);
    double sigmasdLinha = calculaSigmasdLinha(propriedadesMateriais.Es, esLinha, fyd);

    AsLinha = ((mi - miLimite) * dimensoesSecao.b * dimensoesSecao.d * sigmacd) / ((1 - delta) * sigmasdLinha);
    As = ((lambda * qsiLimite) + (((mi - miLimite)) / (1 - delta))) * dimensoesSecao.b * dimensoesSecao.d * (sigmacd / fyd); 
  }

  // 10 - Armadura mínima

  propriedadesMateriais.fck *= 10;
  fyd *= 10;

  double romin, Asmin;
  if (propriedadesMateriais.fck <= 50) {
    romin = (0.078 * std::pow(propriedadesMateriais.fck, (2.0 / 3))) / fyd;
  } else {
    romin = (0.5512 * std::log(1 + 0.11 * propriedadesMateriais.fck)) / fyd;
  }
  if (romin < (0.15 / 100)) {
    Asmin = romin * dimensoesSecao.b * dimensoesSecao.h;
  }

  if (As < Asmin) {
    As = Asmin;
  }
  std::cout << "As: " << As << std::endl << "AsLinha: " <<AsLinha << std::endl;
}






