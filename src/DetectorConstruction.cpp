//
// Created by skyair on 11/20/18.
//
/// \file DetectorConstruction.cpp
/// \brief Implementation of the DetectorConstruction class

// The geometry of the detector is described in this file.

#define DEBUG 1
// TODO: TIDY AND CLEAR DEBUG AREA
#if DEBUG
#include <fstream>
#endif	// DEBUG

#include <DetectorConstruction.h>

#include <G4PVPlacement.hh>
#include <G4Box.hh>
#include <G4SystemOfUnits.hh>
#include <G4NistManager.hh>
#include <G4LogicalVolume.hh>

#include <G4UniformMagField.hh>
#include <G4FieldManager.hh>
#include <G4Tubs.hh>
#include <G4Cons.hh>
#include <G4Orb.hh>

// We need a Ti shell to hold the gas flicker. It has been simplified to a Ti
// window. Neutrons has to go through the Ti window then CCl4.
// This macro toggles if the Ti window exist.
#define TI_WINDOW 1

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() = default;
G4VPhysicalVolume *DetectorConstruction::Construct() {

	// =========================================================================
	// Public Variables

	// Get nist material manager. This is a material library. Your can find 
	// material which you want to use in it. Just by typing
	// nist->FindOrBuildMaterial("<YOUR MATERIAL>")
	// to get your material.
	auto nist = G4NistManager::Instance();

	// This variable is to set if check the overlaps of the volume. If true, the
	// overlaps would be checked.
	bool checkOverlaps = true;

	// Theta represents the angle between the neutron beam and the detector. The 
	// appropriate choice of this angle allows the reduction of the original 
	// neutron effect on the background of the result and does not allow the 
	// number of protons to be too low at the same time.
	double theta = 30.*deg;
	G4RotationMatrix globalRotateMatrix;
	globalRotateMatrix.rotateY(theta);

	// =========================================================================
	// World
	// We need a world to room our particle source, target, detector and so on. 

	/* RM
	double worldLength = 2000.*mm;
	double worldXLength = worldLength;
	double worldYLength = worldLength;
	double worldZLength = worldLength;
	auto *worldMaterial = nist->FindOrBuildMaterial("G4_AIR");
	auto *worldSolid = new G4Box(
		"World",			// Its name.
		worldXLength / 2,	// X Length.
		worldYLength / 2,	// Y Length.
		worldZLength / 2	// Z Length.
	);
	*/
	double worldRadius = 1300.*mm;
	auto worldMaterial = nist->FindOrBuildMaterial("G4_AIR");
	auto worldSolid = new G4Orb(
		"World",	// Name
		worldRadius	// Radius
	);
	auto worldLogic = new G4LogicalVolume(
		worldSolid,		// Its solid.
		worldMaterial,	// Its material.
		"World"         // Its name.
	);
	auto worldPhys = new G4PVPlacement(
		nullptr,			// No rotation.
		G4ThreeVector(),	// At (0,0,0).
		worldLogic,			// Its logical volume.
		"World",			// Its name.
		nullptr,			// Its mother volume. World do not has a 
							// mother volume.
		false,				// No boolean operation.
		0,					// Copy number
		checkOverlaps		// Overlaps checking
	);

	// =========================================================================
	// Target
	// Neutrons hit target and generate proton. This is the target which 
	// neutrons hit. The target was made by PE because PE is full of H.

	double targetRadius = 5.*mm;
	double targetHeight = 0.2*mm;
	auto targetMaterial = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
	auto targetPosition = G4ThreeVector(0.*mm, 0.*mm, 0.*mm);
	auto targetSolid = new G4Tubs(
		"Target",			// Its name.
		0.,					// Minimum radius. 0 means it is solid.
		targetRadius,		// Maximum radius.
		targetHeight / 2,	// Half length. Thus, the targetHeight has 
							// been divided by 2.
		0.*deg,				// Start angle.
		360.*deg			// End angle.
	);
	auto targetLogic = new G4LogicalVolume(
		targetSolid,    // Its solid.
		targetMaterial,	// Its material.
		"Target"        // Its name.
	);
	auto targetPhys = new G4PVPlacement(
		G4Transform3D(
			globalRotateMatrix,
			targetPosition
		),				// Position and rotation.
		targetLogic,    // Its logical volume.
		"Target",       // Its name.
		worldLogic,     // Its mother volume.
		false,          // No boolean operation.
		0,              // Copy number.
		checkOverlaps	// Overlaps checking.
	);

	// =========================================================================
	// Quasi-straight Hole
	// We need an hole to straighten the proton which generated by the collision
	// between the neutrons and PE. Straighter 1 is a cone made by iron and 
	// straighter 2 is a cylinder made by lead. These straighter can not only 
	// absorb the proton but also absorb the neutron which not collide with PE.

	double straighter1Distance = 70.*mm;
	double straighter2Distance = 90.*mm;
	double straighterHoleRadius = 5.*mm;
	double straighterSmallRadius = 15.*mm;
	double straighterBigRadius = 20.*mm;
	double straighter1Height = 20.*mm;
	double straighter2Height = 20.*mm;
	auto straighter1Position = G4ThreeVector(
		(straighter1Distance - straighter1Height / 2)*std::sin(theta),	// X
		0.*mm,															// Y
		(straighter1Distance - straighter1Height / 2)*std::cos(theta)	// Z
	);
	auto straighter2Position = G4ThreeVector(
		(straighter2Distance - straighter2Height / 2)*std::sin(theta),	// X
		0.*mm,															// Y
		(straighter2Distance - straighter2Height / 2)*std::cos(theta)	// Z
	);
	auto straighter1Material = nist->FindOrBuildMaterial("G4_Fe");
	auto straighter2Material = nist->FindOrBuildMaterial("G4_Pb");
	auto straighter1Solid = new G4Cons(
		"Straighter1",			// Name.
		straighterHoleRadius,	// Hole Radius of top.
		straighterSmallRadius,	// Radius of top.
		straighterHoleRadius,	// Hole radius of bottom.
		straighterBigRadius,	// Radius of bottom.
		straighter1Height / 2,	// Half Length.
		0.*deg,					// Start angle.
		360.*deg				// End angle.
	);
	auto straighter1Logic = new G4LogicalVolume(
		straighter1Solid,		// Its solid.
		straighter1Material,	// Its material.
		"Straighter1"			// Its name.
	);
	auto straighter1Phys = new G4PVPlacement(
		G4Transform3D(globalRotateMatrix, straighter1Position),
		straighter1Logic,	// Its logical volume.
		"Straighter1",		// Its name.
		worldLogic,			// Its mother volume.
		false,				// No boolean operation.
		0,					// Copy number
		checkOverlaps		// Overlaps checking
	);
	auto straighter2Solid = new G4Tubs(
		"Straighter2",          // Name.
		straighterHoleRadius,   // Hole Radius.
		straighterBigRadius,	// Radius.
		straighter2Height / 2,	// Half Length.
		0.*deg,					// Start angle.
		360.*deg				// End angle.
	);
	auto straighter2Logic = new G4LogicalVolume(
		straighter2Solid,		// Its solid.
		straighter2Material,	// Its material.
		"Straighter2"			// Its name.
	);
	auto straighter2Phys = new G4PVPlacement(
		G4Transform3D(globalRotateMatrix, straighter2Position),
		straighter2Logic,	// Its logical volume.
		"Straighter2",		// Its name.
		worldLogic,			// Its mother volume.
		false,				// No boolean operation.
		0,					// Copy number
		checkOverlaps		// Overlaps checking
	);

	// =========================================================================
	// Ti Window
#if TI_WINDOW
	// TODO: Study what is "Tiwindow". Is it essential for us? If so, write it.

	double TiWindowDistance = 100.*mm;
	double TiWindowRadius = 10.*mm;
	double TiWindowHeight = 0.02*mm;
	auto TiWindowMaterial = nist->FindOrBuildMaterial("G4_Ti");
	auto TiWindowPosition = G4ThreeVector(
		(TiWindowDistance - TiWindowHeight / 2)*std::sin(theta),
		0.*mm,
		(TiWindowDistance - TiWindowHeight / 2)*std::cos(theta)
	);
	auto TiWindowSolid = new G4Tubs(
		"TiWindow",			// Name
		0.*mm,				// Inner radius
		TiWindowRadius,		// Outer radius
		TiWindowHeight / 2,	// Height
		0.*deg,				// Start angle
		360.*deg			// End angle
	);
	auto TiWindowLogic = new G4LogicalVolume(
		TiWindowSolid,		// Solid
		TiWindowMaterial,	// Material
		"TiWindow"			// Name
	);
	auto TiWindowPhys = new G4PVPlacement(
		G4Transform3D(globalRotateMatrix, TiWindowPosition),
		TiWindowLogic,	// Logical Volume
		"TiWindow",		// Name
		worldLogic,		// Mother volume
		false,			// Boolean operation
		0,				// Copy number
		checkOverlaps	// Overlaps checking
	);
#endif // TI_WINDOW

	// =========================================================================
	// Detector
	// The detector is cylindrical. It is filled with 90% argon and 10% carbon 
	// tetrafluoride. Although Carbon tetrafluoride has a good scintillation 
	// effect, it will cause serious environmental problems. Therefore, argon is
	// filled to reduce the content of tetrafluoride.

	double detectorDistance = 1100.*mm;
	double detectorRadius = 50.*mm;
	double detectorHeight = 1000.*mm;
	// ============================Ar_0.9-CF4_0.1===============================
	// TODO: TIDY THE CODE BLOCK
	G4String name, symbol;             // a=mass of a mole;
	G4double a, z, density;            // z=mean number of protons;
	G4int iz, n;                       // iz=nb of protons  in an isotope;
									   // n=nb of nucleons in an isotope;
	G4int ncomponents, natoms;
	G4double abundance, fractionmass;
	G4double temperature, pressure;
	auto state = kStateGas;

	auto detectorMaterial = new G4Material(
		name = "Ar_0.9-CF4_0.1",
		density = 0.00798*g / cm3,	// 4 atm
		ncomponents = 3,
		state = kStateGas,
		temperature = 293.15*kelvin,
		pressure = 4 * atmosphere
	);
	detectorMaterial->AddElement(nist->FindOrBuildElement("Ar"), natoms = 9);
	detectorMaterial->AddElement(nist->FindOrBuildElement("C"), natoms = 1);
	detectorMaterial->AddElement(nist->FindOrBuildElement("F"), natoms = 4);

	// ============================Ar_0.9-CF4_0.1===============================

	auto detectorPosition = G4ThreeVector(
		(detectorDistance - detectorHeight / 2)*std::sin(theta),
		0.*mm,
		(detectorDistance - detectorHeight / 2)*std::cos(theta)
	);
	auto detectorSolid = new G4Tubs(
		"Detector",			// Name
		0.*mm,				// Inner radius
		detectorRadius,		// Outer radius
		detectorHeight / 2,	// Height
		0.*deg,				// Start angle
		360.*deg			// End angle
	);
	auto detectorLogic = new G4LogicalVolume(
		detectorSolid,		// Solid
		detectorMaterial,	// Material
		"Detector"			// Name
	);
	auto detectorPhys = new G4PVPlacement(
		G4Transform3D(globalRotateMatrix, detectorPosition),
		detectorLogic,	// Logical Volume
		"Detector",		// Name
		worldLogic,		// Mother volume
		false,			// Boolean operation
		0,				// Copy number
		checkOverlaps	// Overlaps checking
	);

#if DEBUG
	std::ofstream debug;
	debug.open(R"(C:\Users\icedr\Desktop\debug.txt)");
	debug << *(G4Isotope::GetIsotopeTable()) << std::endl;
	debug << *(G4Element::GetElementTable()) << std::endl;
	debug << *(G4Material::GetMaterialTable()) << std::endl;
	debug.close();
#endif	// DEBUG
	// =========================================================================

	// The method always return physical world. This is a rule for Geant4.
	return worldPhys;
}
