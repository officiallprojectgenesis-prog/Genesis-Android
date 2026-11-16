# Próximo lote Lua: estado de peds e armamento

Prioridade revista em 2026-11-15 após inspecionar os scripts client/shared dos
recursos disponíveis em `backend_mta/mods/deathmatch/resources`.
A seleção considera chamadas reais no código, e não somente a quantidade de
funções ausentes no catálogo. As cinco funções continuam stubs neste lote.

## Evidência de uso

| Função | Recursos / arquivos / chamadas estáticas, todos os recursos | Recursos / arquivos / chamadas, grupo WorldRPG |
|---|---|---|
| `isPedDead` | 15 / 19 / 37 | 5 / 6 / 18 |
| `getPedWeapon` | 9 / 10 / 20 | 2 / 3 / 8 |
| `getPedTotalAmmo` | 5 / 5 / 10 | 1 / 1 / 5 |
| `getPedWeaponSlot` | 3 / 3 / 4 | 1 / 1 / 1 |
| `getPedAmmoInClip` | 2 / 2 / 2 | 1 / 1 / 1 |

O recurso `native-ped-traffic/client.lua:1039-1054` usa arma e munição na
verificação de convergência do estado dos grupos de NPCs. Retornar zero
constantemente impede esse diagnóstico de representar o estado real.
A mesma verificação consulta `getPedStat`, que já possui handler; seu backend
também precisa ser conferido antes de considerar a validação de armas completa.

A auditoria leu **312 scripts declarados** client/shared no conjunto inteiro e
**38** no grupo WorldRPG. Encontrou **25** e **16** problemas de metadados/arquivos,
respectivamente; nenhum arquivo Lua compilado foi contado. Portanto, o recorte
é parcial. Comentários, strings, declarações de funções e scripts server foram
excluídos. Aliases, OOP, nomes locais que ocultam APIs e código gerado não são resolvidos. Os números medem
chamadas estáticas, sem confirmar recursos ativos ou frequência em execução.
A classificação usa o catálogo regenerado com a revisão GUI/DX e o lote
HUD/RPC commitado em `c1aaeda`; chamadas de controle já implementadas nesse
lote saíram da lista de stubs. As cinco consultas de ped continuam pendentes.

## Ordem de implementação

1. **Backend comum de consulta de ped:** separar jogador local, ped remoto e
   estado de cliente preservado quando o corpo não está carregado. Ler o corpo
   no game thread e aproveitar o estado dos RPCs de arma; não acessar um CPed
   livre nem fabricar zero quando faltam dados.
2. **`isPedDead`:** portar `IsDead() || IsDying()` e a janela de morte na rede,
   incluindo a recuperação após reviver. Saúde menor ou igual a zero, isoladamente,
   não reproduz a função desktop. Referência: `CLuaPedDefs.cpp:2168-2198`.
3. **`getPedWeaponSlot` e `getPedWeapon`:** slot atual e slot opcional, tipo
   de arma e retorno `false` para argumentos inválidos. Referências:
   `CLuaPedDefs.cpp:685-731`.
4. **`getPedTotalAmmo` e `getPedAmmoInClip`:** estoque por slot, munição no
   pente, slots sem munição e o cache para ped não carregado. Preservar as
   conversões `unsigned short` e a falha indicada pelo desktop. Referências:
   `CLuaPedDefs.cpp:733-807`.

Antes de fechar o lote: testar jogador/ped, slot atual/expresso/inválido,
stream-in/out, morte/respawn, RPC de dar/tirar arma, recarga e remoção/reuso do
ID. A parte de estado pode ser testada no host; corpo GTA e inventário precisam
ser conferidos no aparelho. Só marcar uma função como convertida quando a
consulta tiver dados reais e ciclo de vida correto.

## Lotes seguintes

| Ordem | Funções / área | Dependências |
|---|---|---|
| 2 | `getElementsWithinRange`, `createColSphere`, `createColCuboid`, `isElementWithinColShape`, `getElementsWithinColShape` | Preservar geometria de colshape hoje descartada em ENTITY_ADD; manter colisores com posição atual, interior/dimensão, remoção e eventos hit/leave. No inventário atual só `getElementsWithinRange` (2 recursos) e `createColSphere` (1) tiveram chamadas globais detectadas. |
| 3 | `acquireElementStreamingLease`, `releaseElementStreamingLease` | Aparecem em 5 recursos WorldRPG cada, com 27 e 21 chamadas. Portar referência/streaming real e liberação no stop; um token local sem impedir stream-out não resolve a dependência. |
| 4 | `call`, depois `downloadFile` | `call` apareceu em 6 recursos; exige exports e isolamento dos ambientes. Downloads exigem conclusão real, evento e cancelamento por geração do recurso. |
| 5 | `engineLoadTXD`, `engineImportTXD`, `engineLoadDFF`, `engineReplaceModel` | Alto impacto para mapas e skins; precisam de streaming, restauração e validação no aparelho. Não tiveram chamadas globais ausentes detectadas neste inventário; aliases/OOP podem ocultar uso. |

## Reproduzir a busca

```sh
python3 tools/audit_lua_client_usage.py /caminho/do/servidor/mods/deathmatch/resources --json-out /tmp/client-api-usage.json
python3 tools/audit_lua_client_usage.py '/caminho/do/servidor/mods/deathmatch/resources/[worldrpg]' --json-out /tmp/worldrpg-api-usage.json
```

O comando só lê fontes/metadados. O JSON inclui exemplos com arquivo/linha e
os erros de leitura para que o recorte possa ser conferido. A cobertura GUI/DX
199/199 continua sendo de handlers, com as limitações visuais e nativas
registradas no roadmap.
