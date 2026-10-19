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

export const MUJ_CAMPUS_CENTER: [number, number] = [75.56540, 26.84360]; // [lng, lat]
export const MUJ_DEFAULT_ZOOM = 16.4;

export const CAMPUS_VENUES: CampusVenueGeo[] = [
  {
    id: 0,
    label: 'Main Gate',
    fullName: 'Main Gate — Primary campus arrival & visitor entry',
    coordinates: [75.56535, 26.84160],
    category: 'arrival',
    landmarkIndex: null,
    confidence: 'approximate',
    notes: 'South vehicular/pedestrian entry gate off Jaipur-Ajmer Expressway (NH-8). Inferred from user annotated reference map.',
  },
  {
    id: 1,
    label: 'Dome Building',
    fullName: 'Dome Building — University administrative focal point & central dome',
    coordinates: [75.56550, 26.84220],
    category: 'academic',
    landmarkIndex: 1,
    confidence: 'approximate',
    notes: 'Iconic central dome building of MUJ, mapped on the building footprint labelled Administrative Block on OSM basemap.',
  },
  {
    id: 2,
    label: 'Grand Stairs',
    fullName: 'Grand Stairs / Central Plaza — Central arrival and circulation hub',
    coordinates: [75.56535, 26.84275],
    category: 'hub',
    landmarkIndex: null,
    confidence: 'approximate',
    notes: 'Pedestrian steps and circulation plaza immediately north/above the Dome Building / Administrative Block.',
  },
  {
    id: 3,
    label: 'Academic Block 1',
    fullName: 'Academic Block 1 — West quadrangle complex (below AB3)',
    coordinates: [75.56450, 26.84310],
    category: 'academic',
    landmarkIndex: 2,
    confidence: 'approximate',
    notes: 'Major teaching complex with courtyard, situated left/west of Grand Stairs and south of Academic Block 3.',
  },
  {
    id: 4,
    label: 'Academic Block 2',
    fullName: 'Academic Block 2 — East teaching and engineering quadrangle',
    coordinates: [75.56610, 26.84290],
    category: 'academic',
    landmarkIndex: 3,
    confidence: 'approximate',
    notes: 'Major academic quadrangle east of the central axis, south of TMA Pai Auditorium.',
  },
  {
    id: 5,
    label: 'Old Mess / Food Court',
    fullName: 'Old Mess / Food Court — Central dining and student commons',
    coordinates: [75.56530, 26.84360],
    category: 'dining',
    landmarkIndex: 4,
    confidence: 'approximate',
    notes: 'Central campus dining facility located along the central pedestrian axis just north of Grand Stairs.',
  },
  {
    id: 6,
    label: 'Academic Block 3',
    fullName: 'Academic Block 3 — North-west academic building (above AB1)',
    coordinates: [75.56430, 26.84400],
    category: 'academic',
    landmarkIndex: 6,
    confidence: 'approximate',
    notes: 'North-west academic complex placed above Academic Block 1 along the western campus spine.',
  },
  {
    id: 7,
    label: 'Lecture Hall Complex',
    fullName: 'Lecture Hall Complex — Tiered auditorium-style lecture halls (LHC)',
    coordinates: [75.56500, 26.84435],
    category: 'academic',
    landmarkIndex: 5,
    confidence: 'approximate',
    notes: 'Tiered lecture halls complex located north of Old Mess along the central campus road. Searchable via alias LHC.',
  },
  {
    id: 8,
    label: 'Workshops / Labs',
    fullName: 'Workshops / Labs — Eastern engineering and research workshops',
    coordinates: [75.56660, 26.84370],
    category: 'academic',
    landmarkIndex: 8,
    confidence: 'approximate',
    notes: 'Large mechanical and civil engineering workshops complex on the eastern campus perimeter.',
  },
  {
    id: 9,
    label: 'Football Ground',
    fullName: 'Football Ground — Campus sports turf and athletic field',
    coordinates: [75.56590, 26.84460],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'approximate',
    notes: 'Regulation sports turf located east of the Lecture Hall Complex as indicated by red arrow annotation.',
  },
  {
    id: 10,
    label: 'Cricket Ground',
    fullName: 'Cricket Ground — Northern circular cricket field',
    coordinates: [75.56420, 26.84550],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'approximate',
    notes: 'Circular cricket field forming the northern sports ground boundary of the academic campus.',
  },
  {
    id: 11,
    label: 'Outdoor Courts',
    fullName: 'Outdoor Courts — Basketball, tennis & volleyball courts',
    coordinates: [75.56520, 26.84580],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'approximate',
    notes: 'Outdoor sports courts complex located east of Cricket Ground and north of the main campus avenue.',
  },
  {
    id: 12,
    label: 'Knowledge Centre',
    fullName: 'Knowledge Centre — Digital library & resource centre',
    coordinates: [75.56640, 26.84230],
    category: 'library',
    landmarkIndex: null,
    confidence: 'approximate',
    notes: 'Dedicated digital library and resource center situated to the right/east of Dome Building / Administrative Block.',
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
