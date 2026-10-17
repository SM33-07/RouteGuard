/**
 * RouteGuard — MUJ Campus Verified Venue Geographic Layout
 *
 * Provides calibrated [longitude, latitude] coordinates for the 13 C-owned campus venues.
 * Coordinates are mapped around the central MUJ campus benchmark (approx. 75.565234, 26.843855)
 * using Google Maps satellite reference imagery (docs/reference, public/maps) and official master plans.
 *
 * VENUE ACCURACY BOUNDARY:
 * Connections are synthetic/modeled for CSC2101/CSC2130 DSA graph demonstration.
 * They do NOT represent verified pedestrian sidewalks, municipal transit, or emergency paths.
 */

import type { FeatureCollection, Point } from 'geojson';

export interface CampusVenueGeo {
  id: number;
  label: string;
  fullName: string;
  coordinates: [number, number]; // [longitude, latitude] (WGS 84 / GeoJSON format)
  category: 'arrival' | 'academic' | 'dining' | 'sports' | 'library' | 'hub';
  landmarkIndex: number | null; // Corresponding 1-8 marker on schematic, if applicable
  confidence: 'verified_poi' | 'approximate';
  notes: string;
}

export const MUJ_CAMPUS_CENTER: [number, number] = [75.565234, 26.843855]; // [lng, lat]
export const MUJ_DEFAULT_ZOOM = 16.4;

export const CAMPUS_VENUES: CampusVenueGeo[] = [
  {
    id: 0,
    label: 'Main Gate',
    fullName: 'Main Gate — Primary campus arrival & visitor entry',
    coordinates: [75.56535, 26.84160],
    category: 'arrival',
    landmarkIndex: null,
    confidence: 'verified_poi',
    notes: 'South vehicular/pedestrian entry gate off Jaipur-Ajmer Expressway (NH-8).',
  },
  {
    id: 1,
    label: 'Dome Building',
    fullName: 'Dome Building — Library, administration, services (Landmark 1)',
    coordinates: [75.56575, 26.84285],
    category: 'academic',
    landmarkIndex: 1,
    confidence: 'verified_poi',
    notes: 'Architectural focal point with central dome, university administrative offices and student services.',
  },
  {
    id: 2,
    label: 'Grand Stairs',
    fullName: 'Grand Stairs / Central Plaza — Central arrival and circulation hub',
    coordinates: [75.56530, 26.84340],
    category: 'hub',
    landmarkIndex: null,
    confidence: 'approximate',
    notes: 'Central pedestrian circulation plaza between Dome Building and Academic Blocks 1 & 2.',
  },
  {
    id: 3,
    label: 'Academic Block 1',
    fullName: 'Academic Block 1 — Left side of the central stairs (Landmark 2)',
    coordinates: [75.56430, 26.84390],
    category: 'academic',
    landmarkIndex: 2,
    confidence: 'verified_poi',
    notes: 'Major teaching complex with interior quadrangle/courtyard, west of central spine.',
  },
  {
    id: 4,
    label: 'Academic Block 2',
    fullName: 'Academic Block 2 — Right side of the central stairs (Landmark 3)',
    coordinates: [75.56620, 26.84410],
    category: 'academic',
    landmarkIndex: 3,
    confidence: 'verified_poi',
    notes: 'Major teaching complex east of the central spine, housing engineering wings and AIC-MUJ.',
  },
  {
    id: 5,
    label: 'Old Mess / Food Court',
    fullName: 'Old Mess / Food Court — Dining and student seating (Landmark 4)',
    coordinates: [75.56515, 26.84415],
    category: 'dining',
    landmarkIndex: 4,
    confidence: 'verified_poi',
    notes: 'Central campus dining facility located between 1AB and 2AB along the central axis.',
  },
  {
    id: 6,
    label: 'Academic Block 3',
    fullName: 'Academic Block 3 — Arts, media and specialist spaces (Landmark 6)',
    coordinates: [75.56410, 26.84490],
    category: 'academic',
    landmarkIndex: 6,
    confidence: 'verified_poi',
    notes: 'North-west academic building accommodating architecture, media, design, and research labs.',
  },
  {
    id: 7,
    label: 'Lecture Hall Complex',
    fullName: 'Lecture Hall Complex — Large teaching halls (Landmark 5)',
    coordinates: [75.56525, 26.84510],
    category: 'academic',
    landmarkIndex: 5,
    confidence: 'verified_poi',
    notes: 'Tiered auditorium-style lecture halls north of Old Mess.',
  },
  {
    id: 8,
    label: 'Workshops / Labs',
    fullName: 'Workshops / Labs — Workshop-side teaching facilities (Landmark 8)',
    coordinates: [75.56710, 26.84460],
    category: 'academic',
    landmarkIndex: 8,
    confidence: 'verified_poi',
    notes: 'Mechanical, electrical and civil engineering workshops on the eastern campus edge.',
  },
  {
    id: 9,
    label: 'Football Ground',
    fullName: 'Football Ground — Sports grounds (Landmark 7)',
    coordinates: [75.56630, 26.84590],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'verified_poi',
    notes: 'Full-size regulation football turf north-east of the Lecture Hall Complex.',
  },
  {
    id: 10,
    label: 'Cricket Ground',
    fullName: 'Cricket Ground — Sports grounds (Landmark 7)',
    coordinates: [75.56500, 26.84680],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'verified_poi',
    notes: 'Circular cricket field forming the northern sports boundary of the academic campus.',
  },
  {
    id: 11,
    label: 'Outdoor Courts',
    fullName: 'Outdoor Courts / Turfs — Sports grounds (Landmark 7)',
    coordinates: [75.56740, 26.84320],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'verified_poi',
    notes: 'Tennis, basketball and volleyball outdoor courts southeast near the hostel boundary.',
  },
  {
    id: 12,
    label: 'Knowledge Centre',
    fullName: 'Knowledge Centre — Digital library & resource centre',
    coordinates: [75.56655, 26.84345],
    category: 'library',
    landmarkIndex: null,
    confidence: 'approximate',
    notes: 'Curved modern glass library building situated northeast of the Dome Building and south of 2AB along Vinayak Marg.',
  },
];

/**
 * Returns GeoJSON FeatureCollection of venue points
 */
export function getCampusVenuesGeoJSON(): FeatureCollection<Point> {
  return {
    type: 'FeatureCollection',
    features: CAMPUS_VENUES.map((v) => ({
      type: 'Feature',
      id: v.id,
      properties: {
        id: v.id,
        label: v.label,
        fullName: v.fullName,
        category: v.category,
        landmarkIndex: v.landmarkIndex,
        confidence: v.confidence,
        notes: v.notes,
      },
      geometry: {
        type: 'Point',
        coordinates: v.coordinates,
      },
    })),
  };
}
