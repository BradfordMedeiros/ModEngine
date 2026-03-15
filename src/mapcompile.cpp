#include "./mapcompile.h"

struct RailEntity {
  glm::vec3 position;
  glm::vec3 rotation; // euler angles
  std::string railName;
  int railIndex;
  int railTime;
};
struct OrbEntity {
  glm::vec3 position;
  glm::vec3 rotation;  // euler angles
  std::string orbName;
  std::string orbUi;
  std::string level;
  std::vector<std::string> conn;
};

std::vector<int> getOrbConnectionIndex(std::vector<OrbEntity>& orbs, int index){
  std::vector<int> connections;
  OrbEntity& orbEntity = orbs.at(index);
  for (int i = 0; i < orbEntity.conn.size(); i++){
    auto name = orbEntity.conn.at(i);
    bool matchedOrbName = false;
    for (int j = 0; j < orbs.size(); j++){
      if (name == orbs.at(j).orbName && orbEntity.orbUi == orbs.at(j).orbUi){
        connections.push_back(j);
        matchedOrbName = true;
        break;
      }
    }
    if (!matchedOrbName){
      modassert(false, std::string("no matching orb name: ") + name);
    }
  }
  return connections;
}

void addTriggerColor(Entity& entity, std::vector<GameobjAttributeOpts>& attributes){
  auto triggercolor = getValue(entity, "triggercolor");
  if (triggercolor.has_value()){
    attributes.push_back(GameobjAttributeOpts {
     .field = "triggercolor",
     .attributeValue = *triggercolor.value(),
    });  

    auto activeColor = getVec4Value(entity, "activecolor");
    auto unactiveColor = getVec4Value(entity, "unactivecolor");
    if (activeColor.has_value()){
      attributes.push_back(GameobjAttributeOpts {
       .field = "activecolor",
       .attributeValue = activeColor.value(),
      });  
    }
    if (unactiveColor.has_value()){
      attributes.push_back(GameobjAttributeOpts {
       .field = "unactivecolor",
       .attributeValue = unactiveColor.value(),
      });  
    }
  }
}

void addCoreTrench(Entity& entity, std::vector<GameobjAttributeOpts>& attributes, std::string meshpath){
  attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
    .field = "mesh",
    .attributeValue = meshpath,
  });
  attributes.push_back(GameobjAttributeOpts {
    .field = "physics_shape",
    .attributeValue = "shape_exact",
  });
  attributes.push_back(GameobjAttributeOpts {
    .field = "physics",
    .attributeValue = "enabled",
  });

  addTriggerColor(entity, attributes);
}

struct BallGameCompile {
  std::vector<RailEntity> rails;
  std::vector<OrbEntity> orbs;
};

CompileMapFns getCompileMapForGame(){
  auto ballGameCompileSharedPtr = std::make_shared<BallGameCompile>();
  auto compileFn = [ballGameCompileSharedPtr](std::string& brushFileOut, MapData& mapData, Entity& entity, bool* shouldWrite, std::vector<GameobjAttributeOpts>& attributes, std::string* modelName) -> void {
    auto& ballGameCompile = *ballGameCompileSharedPtr;
    auto origin = getValue(entity, "origin");
    auto className = getValue(entity, "classname");

    modassert(className.has_value(), std::string("no className index = ") + std::to_string(entity.index));
    std::cout << "compile index: " << entity.index << std::endl;
    std::cout << "origin: " << (origin.has_value() ? *origin.value() : "no origin") << std::endl;
  
    int layerIndex = -1;
    if (isLayerEntity(entity, &layerIndex) && entity.brushes.size() > 0){
      // same as world spawn, but without added keys
      *shouldWrite = true;

      addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".brush");

    }else if (*className.value() == "player_start"){
      *modelName = "playerspawn";
      *shouldWrite = true;
    }else if (*className.value() == "powerup_jump" || *className.value() == "powerup_dash" || *className.value() == "powerup_teleport" || *className.value() == "powerup_lowgravity"){
      *shouldWrite = true;

      attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
        .field = "scale",
        .attributeValue = glm::vec3(1.f, 1.f, 1.f),
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_shape",
        .attributeValue = "shape_sphere",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics",
        .attributeValue = "enabled",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "physics_collision",
        .attributeValue = "nocollide",
      });

      // Replace with a better powerup model
      attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
        .field = "mesh",
        .attributeValue = "./res/models/box/crate.gltf",
      });

      // When it becomes deactivated (respawnable but used) make it transparent
      // So hence this layer
      attributes.push_back(GameobjAttributeOpts {
        .field = "layer",
        .attributeValue = "transparency",
      });

      auto rate = getIntValue(entity, "rate");
      if (rate.has_value()){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup-rate",
          .attributeValue = static_cast<float>(rate.value()),
        });
      }

      if (*className.value() == "powerup_jump"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "jump",
        });          
      }else if (*className.value() == "powerup_dash"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "dash",
        });          
      }else if (*className.value() == "powerup_teleport"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "low_gravity",
        });          
      }else if (*className.value() == "powerup_lowgravity"){
        attributes.push_back(GameobjAttributeOpts {
          .field = "powerup",
          .attributeValue = "teleport",
        });          
      }else{
        modassert(false, "invalid powerup type");
      }

    }else if (*className.value() == "vertical_bound_plane"){
      *shouldWrite = true;

      double yValueSum = 0;
      int totalPoints = 0;

      for (auto& brush : entity.brushes){
        for (auto &brushFace : brush.brushFaces){
          yValueSum += brushFace.point1.y;
          yValueSum += brushFace.point2.y;
          yValueSum += brushFace.point3.y;
          totalPoints += 3;
        }
      }
        
      modassert(totalPoints > 0, "invalid ballplane no faces");
      auto average = yValueSum / totalPoints;
      modassert(average > -10000 && average < 10000, "invalid ballplane"); // arbitrary numbers to guard against weird

      attributes.push_back(GameobjAttributeOpts {
        .field = "ballplane",
        .attributeValue = "true",
      });
      attributes.push_back(GameobjAttributeOpts {
        .field = "position", 
        .attributeValue = glm::vec3(0.f, average, 0.f),
      });
    }else if (*className.value() == "worldspawn"){
      *shouldWrite = true;

      addCoreTrench(entity, attributes, brushFileOut);

      for (auto &keyValue : entity.keyValues){
        if (keyValue.key == "_tb_textures" || keyValue.key == "classname" || keyValue.key == "_tb_def"){
          continue;
        }
        std::cout << "compile: worldspawn key: " << keyValue.key << ", value: " << keyValue.value << std::endl;
        attributes.push_back(GameobjAttributeOpts {
          .field = keyValue.key, 
          .attributeValue = keyValue.value,
        });
      }

    }else if (*className.value() == "player_end"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".brush",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "player_end",
          .attributeValue = "true",
        });

    }else if (*className.value() == "trigger_zone"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".brush",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        auto triggerTarget = getValue(entity, "trigger");
        if (triggerTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger_zone",
            .attributeValue = *triggerTarget.value(),
          });
        }

        auto triggerData = getValue(entity, "trigger_data");
        if (triggerData.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger_data",
            .attributeValue = *triggerData.value(),
          });
        }
        addTriggerColor(entity, attributes);
    }else if (*className.value() == "killplane"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".brush");
        attributes.push_back(GameobjAttributeOpts {
          .field = "killplane",
          .attributeValue = "true",
        });
        
    }else if (*className.value() == "laser"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, "../gameresources/build/objtypes/spawnpoint.gltf");
        attributes.push_back(GameobjAttributeOpts {
          .field = "tint",
          .attributeValue = glm::vec4(0.f, 1.f, 0.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "scale",
          .attributeValue = glm::vec3(5.f, 5.f, 5.f),
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "laser",
          .attributeValue = "true",
        });   

        // this is wrong
        auto rotationEuler = getVec3Value(entity, "angles");
        if (rotationEuler.has_value()){
          auto rotation = quatFromTrenchBroomAngles(
            rotationEuler.value().x,
            rotationEuler.value().y,
            rotationEuler.value().z
          );
          auto vecValue = serializeQuatToVec4(rotation);
          attributes.push_back(GameobjAttributeOpts {
            .field = "rotation",
            .attributeValue = vecValue,
          });
        }

        auto laserLength = getScaledFloatValue(mapData, entity, "length");
        attributes.push_back(GameobjAttributeOpts {
          .field = "laserlength",
          .attributeValue = laserLength.value(),
        });       

        ///////////////////////////////

    }else if (*className.value() == "bouncepad"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".brush");
        auto magnitude = getIntValue(entity, "mag");
        attributes.push_back(GameobjAttributeOpts {
          .field = "bounce",
          .attributeValue = glm::vec3(0.f, 0.f, -1.f * (magnitude.has_value() ? magnitude.value() : 1)),
        });
    }else if (*className.value() == "conveyer"){
        *shouldWrite = true;
        addCoreTrench(entity, attributes, brushFileOut + "," + std::to_string(entity.index) + ".brush");
        auto modspeed = getVec3Value(entity, "move");
        attributes.push_back(GameobjAttributeOpts {
          .field = "modspeed",
          .attributeValue = modspeed.value(),
        });

    }else if (*className.value() == "water"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".brush",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_box",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

    }else if (*className.value() == "teleport_zone"){
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".brush",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        auto teleportTarget = getValue(entity, "exit");
        if (teleportTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "teleport_zone",
            .attributeValue = *teleportTarget.value(),
          });
        }
    }else if (*className.value() == "teleport_exit"){
        std::cout << "compile map unrecognized type: " << *className.value() << std::endl;
        *shouldWrite = true;
        auto teleportTarget = getValue(entity, "exit");
        if (teleportTarget.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "teleport",
            .attributeValue = "true",
          });
          attributes.push_back(GameobjAttributeOpts {
            .field = "teleport_exit",
            .attributeValue = *teleportTarget.value(),
          });             
        }
    }else if (*className.value() == "dynamic"){
        std::cout << "got dynamic" << std::endl;
        *shouldWrite = true;
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = brushFileOut + "," + std::to_string(entity.index) + ".brush",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_exact",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });

        auto curve = getValue(entity, "curve");
        modassert(curve.has_value(), "curve does not have a value");
        attributes.push_back(GameobjAttributeOpts {
          .field = "curve",
          .attributeValue = *curve.value(),
        });

        auto trigger = getValue(entity, "trigger");
        if (trigger.has_value()){
          attributes.push_back(GameobjAttributeOpts {
            .field = "trigger",
            .attributeValue = *trigger.value(),
          });
        }

    }else if (*className.value() == "rail"){
        auto position = getScaledVec3Value(mapData, entity, "origin");
        if (!position.has_value()){
          modassert(false, "rail - position does not have a value");
        }
        auto rail = getValue(entity, "rail");
        modassert(rail.has_value(), "rail does not have a value");

        auto railIndex = getIntValue(entity, "rail-index");
        modassert(railIndex.has_value(), "rail-index does not have a value");

        auto railTime = getIntValue(entity, "time");
        auto rotationEuler = getVec3Value(entity, "angles");

        ballGameCompile.rails.push_back(RailEntity {
          .position = position.value(),
          .rotation = rotationEuler.has_value() ? rotationEuler.value() : glm::vec3(0.f, 0.f, 0.f),
          .railName = *rail.value(),
          .railIndex = railIndex.value(),
          .railTime = railTime.has_value() ? railTime.value() : -1,
        });
    }else if (*className.value() == "orb"){
        auto position = getScaledVec3Value(mapData, entity, "origin");
        if (!position.has_value()){
          modassert(false, "orb - position does not have a value");
        }

        auto orb = getValue(entity, "orb");
        modassert(orb.has_value(), "orb does not have a value");

        auto orbUi = getValue(entity, "orbui");
        modassert(orbUi.has_value(), "orbUi does not have a value");

        auto orbLevel = getValue(entity, "level");
        modassert(orbLevel.has_value(), "orb does not have a level");

        auto rotationEuler = getVec3Value(entity, "angles");
        auto rotation = rotationEuler.has_value() ? rotationEuler.value() : glm::vec3(0.f, 0.f, 0.f);

        OrbEntity orbEntity {
          .position = position.value(),
          .rotation = rotation,
          .orbName = *orb.value(),
          .orbUi = *orbUi.value(),
          .level = *orbLevel.value(),
          .conn = {},
        };

        auto conn = getValue(entity, "conn");
        modassert(conn.has_value(), "orb - conn does not have a value");

        auto connections = split(*conn.value(), ',');
        orbEntity.conn = connections;
        ballGameCompile.orbs.push_back(orbEntity);

    }else if (*className.value() == "gem"){
        *shouldWrite = true;

        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "scale",
          .attributeValue = glm::vec3(1.f, 1.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_shape",
          .attributeValue = "shape_sphere",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics",
          .attributeValue = "enabled",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "physics_collision",
          .attributeValue = "nocollide",
        });

        // Replace with a better gem model
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "mesh",
          .attributeValue = "./res/models/box/crate.gltf",
        });
        attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
          .field = "tint",
          .attributeValue = glm::vec4(1.f, 0.f, 0.f, 1.f),
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "layer",
          .attributeValue = "transparency",
        });

        auto gemValue = getValue(entity, "gem");
        modassert(gemValue.has_value(), "gem does not have a value");
        attributes.push_back(GameobjAttributeOpts {
          .field = "gem",
          .attributeValue = *gemValue.value(),
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "condition",
          .attributeValue = "true",
        });

        attributes.push_back(GameobjAttributeOpts {
          .field = "gem-label",
          .attributeValue = *gemValue.value(),
        });

    }else{
        std::cout << "compile map unrecognized type: " << *className.value() << std::endl;
        *shouldWrite = false;
    }

    auto layer = getValue(entity, "layer");
    if (layer.has_value()){
      bool foundLayer = false;
      for (auto& attribute : attributes){
        if (attribute.field == "layer"){
          foundLayer = true;
          break;
        }
      }
      if (!foundLayer){
        attributes.push_back(GameobjAttributeOpts {
          .field = "layer",
          .attributeValue = *layer.value(),
        });          
      }
    }

    auto scroll = getVec3Value(entity, "scroll");
    if (scroll.has_value()){
      bool foundLayer = false;
      for (auto& attribute : attributes){
        if (attribute.field == "scrollspeed"){
          foundLayer = true;
          break;
        }
      }
      if (!foundLayer){
        attributes.push_back(GameobjAttributeOpts {
          .field = "scrollspeed",
          .attributeValue = scroll.value(),
        });          
      }
    }

  };

  auto finalizeFn = [ballGameCompileSharedPtr](MapData& mapData, std::string& generatedScene) -> void {
    auto& ballGameCompile = *ballGameCompileSharedPtr;
    if (ballGameCompile.rails.size() != 0){
        generatedScene += "combined_entities_rail:rail:true\n";

        {
          std::string data = "combined_entities_rail:data-pos:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + serializeVec(ballGameCompile.rails.at(i).position) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-rot:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + serializeVec(ballGameCompile.rails.at(i).rotation) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }


        {
          std::string data = "combined_entities_rail:data-name:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + ballGameCompile.rails.at(i).railName + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-index:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + std::to_string(ballGameCompile.rails.at(i).railIndex) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_rail:data-time:";
          for (int i = 0; i < ballGameCompile.rails.size(); i++){
            data = data + std::to_string(ballGameCompile.rails.at(i).railTime) + ((i == (ballGameCompile.rails.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }
    }
    if (ballGameCompile.orbs.size() != 0){
        generatedScene += "combined_entities_orb:orbui:true\n";

        {
          std::string data = "combined_entities_orb:data-pos:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + serializeVec(ballGameCompile.orbs.at(i).position) + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        {
          std::string data = "combined_entities_orb:data-rot:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + serializeVec(ballGameCompile.orbs.at(i).rotation) + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }



        {
          std::string data = "combined_entities_orb:data-conn:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            std::string connections;
            std::vector<int> connectionsIndex = getOrbConnectionIndex(ballGameCompile.orbs, i);
            for (int j = 0; j < connectionsIndex.size(); j++){
              connections += std::to_string(connectionsIndex.at(j));
              if (j != (connectionsIndex.size() - 1)){
                connections += ".";
              }
            }
            data = data + connections + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        //data-name
        {
        std::string data = "combined_entities_orb:data-name:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + ballGameCompile.orbs.at(i).orbName + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        //data-orbui
        {
        std::string data = "combined_entities_orb:data-orbui:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + ballGameCompile.orbs.at(i).orbUi + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }

        // data-level
        {
          std::string data = "combined_entities_orb:data-level:";
          for (int i = 0; i < ballGameCompile.orbs.size(); i++){
            data = data + ballGameCompile.orbs.at(i).level + ((i == (ballGameCompile.orbs.size() - 1)) ? "\n" : ",");
          }
          generatedScene += data;
        }
    }
  };

  return CompileMapFns {
    .compileFn = compileFn,
    .finalizeFn = finalizeFn,
  };
}
